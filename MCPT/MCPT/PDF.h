#pragma once
#include "common.h"
#include "Hitable.h"

class PDF
{
public:
	virtual float value(const vec3& direction) const = 0;
	virtual vec3 importance_sampling() const = 0;
};

class PDF_cos : public PDF
{
public:
	PDF_cos(const vec3& n) { 
		_frame.build_frame(n);
		if (isnan(_frame[0][0]))
		{
			vec3 tmp(n), tmp2(_frame[0]);
			cout <<"in PDF_cos construct : input-"<< tmp << ", output-" << tmp2 << endl;
		}
	}
	virtual float value(const vec3& direction) const;
	virtual vec3 importance_sampling() const;
private:
	OrthonormalBases _frame;
};

class PDF_cos_n : public PDF
{
public:
	PDF_cos_n(const vec3& n, float Ns) :Ns(Ns){
		_frame.build_frame(n);
		if (isnan(_frame[0][0]))
		{
			vec3 tmp(n), tmp2(_frame[0]);
			cout << "in PDF_cos_n construct : input-" << tmp << ", output-" << tmp2 << endl;
		}
	}
	virtual float value(const vec3& direction) const;
	virtual vec3 importance_sampling() const;
private:
	float Ns;
	OrthonormalBases _frame;
};

class PDF_to_light : public PDF
{
public:
	PDF_to_light(Hitable& obj, const vec3& origin) :
		obj(&obj), origin(origin) {}
	virtual float value(const vec3& direction) const;
	virtual vec3 importance_sampling() const;
private:
	vec3 origin;
	Hitable* obj;
};

class PDF_mix : public PDF
{
public:
	PDF_mix(std::shared_ptr<PDF> p0, std::shared_ptr<PDF> p1) { _p0 = p0; _p1 = p1; }
	virtual float value(const vec3& direction) const;
	virtual vec3 importance_sampling() const;
private:
	std::shared_ptr<PDF> _p0, _p1;
};