#pragma once
#include "common.h"
#include "Hitable.h"

class OrthonormalBases
{
public:
	OrthonormalBases() {};
	inline vec3 operator[](int i) const { return axis[i]; }
	vec3 t() const { return axis[0]; }
	vec3 bn() const { return axis[1]; }
	vec3 n() const { return axis[2]; }
	vec3 local(float a, float b, float c) const
	{
		return a * t() + b * bn() + c * n();
	}
	vec3 local(vec3& a) const
	{
		return a[0] * t() + a[1] * bn() + a[2] * n();
	}
	void build_frame(const vec3& n)
	{
		axis[2] = normalize(n);
		vec3 tmp;
		if (fabs(axis[2][0] > 0.9))
			tmp = vec3(0, 1, 0);
		else
			tmp = vec3(1, 0, 0);
		axis[1] = normalize(cross(axis[2], tmp));
		axis[0] = cross(axis[2], axis[1]);
	}
private:
	vec3 axis[3]; // tangent, bi-normal, normal
};

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