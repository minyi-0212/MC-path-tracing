#include "PDF.h"

inline vec3 random_cos_direction()
{
	float r1 = random_float_0_1(),
		r2 = random_float_0_1(),
		phi = 2 * M_PI * r1;
	return vec3(cos(phi) * 2 * sqrt(r2),
		sin(phi) * 2 * sqrt(r2),
		sqrt(1 - r2));
}

float PDF_cos::value(const vec3& direction) const
{
	if (isnan(_frame[0][0]))
		cout << "pdf_cos frame nan" << endl;
	float cos = dot(normalize(direction), _frame[2]);
	if (cos > 0)
		return cos / M_PI;
	else
		return 0;
}

//#include <iostream>
vec3 PDF_cos::generate_random_d() const
{
	vec3 tmp = random_cos_direction();
	tmp = _frame.local(tmp);
	if (isnan(tmp[0]))
		std::cout << "pdf_cos: " << tmp[0] << " " << tmp[1] << " " << tmp[2] << std::endl;
	return tmp;
}

float PDF_to_light::value(const vec3& direction) const
{
	return obj->pdf_value(origin, direction);
}

vec3 PDF_to_light::generate_random_d() const
{
	vec3 tmp = obj->random(origin);
	if(isnan(tmp[0]))
		std::cout << "pdf_light: " << tmp[0] << " " << tmp[1] << " " << tmp[2] << std::endl;

	return tmp;
}

float PDF_mix::value(const vec3& direction) const
{
	return 0.5*_p0->value(direction) + 0.5*_p1->value(direction);
}

vec3 PDF_mix::generate_random_d() const
{
	if (random_float_0_1() < 0.5)
		return _p0->generate_random_d();
	else
		return _p1->generate_random_d();
}