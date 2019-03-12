#pragma once
#include "ray.h"

inline float ffmin(float a, float b) { return a < b ? a : b; }
inline float ffmax(float a, float b) { return a > b ? a : b; }

class AABB {
public:
	AABB() {}
	AABB(const vec3& a, const vec3& b) { _min = a; _max = b; }

	vec3 min() const { return _min; }
	vec3 max() const { return _max; }
	inline bool hit(const Ray& r, float tmin, float tmax) const
	{
		for (int a = 0; a < 3; a++) {
			float invD = 1.0f / r.direction()[a];
			float t0 = (_min[a] - r.origin()[a]) *invD;
			float t1 = (_max[a] - r.origin()[a]) *invD;
			if (invD < 0.0)
				std::swap(t0, t1);
			tmin = t0 > tmin ? t0 : tmin;
			tmax = t1 < tmax ? t1 : tmax;
			if (tmax <= tmin)
				return false;
		}
		return true;
	}

private:
	vec3 _min;
	vec3 _max;
};

static AABB merge_box(AABB box0, AABB box1) {
	vec3 small(fmin(box0.min()[0], box1.min()[0]),
		fmin(box0.min()[1], box1.min()[1]),
		fmin(box0.min()[2], box1.min()[2]));
	vec3 big(fmax(box0.max()[0], box1.max()[0]),
		fmax(box0.max()[1], box1.max()[1]),
		fmax(box0.max()[2], box1.max()[2]));
	return AABB(small, big);
}