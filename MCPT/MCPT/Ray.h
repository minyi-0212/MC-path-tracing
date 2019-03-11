#pragma once
#include <glm.hpp>
using glm::vec3;
#define random_float_0_1() rand()/double(RAND_MAX)

class Ray
{
public:
	Ray(){}
	Ray(const vec3& o, const vec3& d):o(o), d(d){}
	vec3 origin() const { return o; }
	vec3 direction() const { return d; }
	vec3 point_at_t(float t) const { return o + t * d; }
private:
	vec3 o, d;
};