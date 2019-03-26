#pragma once
#include "common.h"

class Ray
{
public:
	Ray(){}
	Ray(const vec3& o, const vec3& d):o(o), d(normalize(d)){}
	vec3 origin() const { return o; }
	vec3 direction() const { return d; }
	vec3 point_at_t(float t) const { return o + t * d; }
private:
	vec3 o, d;
};