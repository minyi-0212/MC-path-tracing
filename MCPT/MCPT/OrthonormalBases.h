#include "common.h"

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
		if (fabs(axis[2][0]) > 0.9)
			tmp = vec3(0, 1, 0);
		else
			tmp = vec3(1, 0, 0);
		axis[1] = normalize(cross(axis[2], tmp));
		axis[0] = cross(axis[2], axis[1]);
	}
private:
	vec3 axis[3]; // tangent, bi-normal, normal
};