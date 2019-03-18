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
	float cos = dot(normalize(direction), _frame[2]);
	if (cos > 0)
		return cos / M_PI;
	else
		return 0;
}

vec3 PDF_cos::generate_random_d() const
{
	vec3 tmp = random_cos_direction();
	return _frame.local(tmp);
}

float PDF_to_light::value(const vec3& direction) const
{
	return obj->pdf_value(origin, direction);
}

vec3 PDF_to_light::generate_random_d() const
{
	return obj->random(origin);
}