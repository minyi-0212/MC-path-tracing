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

inline vec3 random_cos_n_direction(float Ns)
{
	float r1 = random_float_0_1(),
		r2 = random_float_0_1(),
		phi = 2 * M_PI * r1,
		theta = acos(pow(r2, 1 / (Ns + 1)));
	return vec3(cos(phi) * sin(theta),
		sin(phi) * sin(theta),
		cos(theta));
}

float PDF_cos::value(const vec3& direction) const
{
	if (isnan(_frame[0][0]))
		cout << "pdf_cos frame nan" << endl;
	float cos = dot(direction, _frame[2]);
	if (cos > 0)
		return cos / M_PI;
	else
		return 0;
}

vec3 PDF_cos::importance_sampling() const
{
	vec3 tmp = random_cos_direction();
	tmp = _frame.local(tmp);
	if (isnan(tmp[0]) || length(tmp)==0)
		std::cout << "pdf_cos: " << tmp[0] << " " << tmp[1] << " " << tmp[2] << std::endl;
	return normalize(tmp);
}

float PDF_cos_n::value(const vec3& direction) const
{
	cout << "PDF_cos_n::value" << endl;
	return 0;
}

vec3 PDF_cos_n::importance_sampling() const
{
	vec3 tmp = random_cos_n_direction(Ns);
	//cout << tmp << endl;
	tmp = _frame.local(tmp);
	if (isnan(tmp[0]) || length(tmp) == 0)
		std::cout << "pdf_cos: " << tmp[0] << " " << tmp[1] << " " << tmp[2] << std::endl;
	return normalize(tmp);
}

float PDF_to_light::value(const vec3& direction) const
{
	return obj->pdf_value(origin, direction);
}

vec3 PDF_to_light::importance_sampling() const
{
	vec3 tmp = obj->random(origin);
	if (isnan(tmp[0]))
		std::cout << "PDF_to_light: " << tmp[0] << " " << tmp[1] << " " << tmp[2] << std::endl;

	return normalize(tmp);
}

float PDF_mix::value(const vec3& direction) const
{
	return 0.5*_p0->value(direction) + 0.5*_p1->value(direction);
}

vec3 PDF_mix::importance_sampling() const
{
	if (random_float_0_1() < 0.5)
		return _p0->importance_sampling();
	else
		return _p1->importance_sampling();
}