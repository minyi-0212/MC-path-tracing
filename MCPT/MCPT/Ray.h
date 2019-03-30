#pragma once
#include "common.h"

class Ray
{
public:
	Ray() {}
	Ray(const vec3& o, const vec3& d) :_o(o)
	{
		_d = normalize(d);
	}
	vec3 origin() const { return _o; }
	vec3 direction() const { return _d; }
	vec3 point_at_t(float t) const { return _o + t * _d; }
private:
	vec3 _o, _d;
};