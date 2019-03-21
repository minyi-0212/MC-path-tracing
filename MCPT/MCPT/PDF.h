#pragma once
#include "common.h"
#include "Hitable.h"

class PDF
{
public:
	virtual float value(const vec3& direction) const = 0;
	virtual vec3 generate_random_d() const = 0;
};

class PDF_cos : public PDF
{
public:
	PDF_cos(const vec3& n) { _frame.build_frame(n); }
	virtual float value(const vec3& direction) const;
	virtual vec3 generate_random_d() const;
private:
	OrthonormalBases _frame;
};

class PDF_to_light : public PDF
{
public:
	PDF_to_light(Hitable& obj, const vec3& origin):
		obj(&obj), origin(origin){}
	virtual float value(const vec3& direction) const;
	virtual vec3 generate_random_d() const;
private:
	vec3 origin;
	Hitable* obj;
};

class PDF_mix : public PDF
{
public:
	PDF_mix(PDF* p0, PDF* p1) { _p0 = p0; _p1 = p1; }
	virtual float value(const vec3& direction) const;
	virtual vec3 generate_random_d() const;
private:
	PDF *_p0, *_p1;
};