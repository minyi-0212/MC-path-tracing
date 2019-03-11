#include "Hitable.h"

bool Sphere::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
	/// A:r.origin() B:r.direction() C:center
	/// t*t*dot(B, B) + 2 * t*dot(B, A - C) + dot(A - C, A - C) - R * R = 0;
	vec3 CA = r.origin() - center;
	float a = dot(r.direction(), r.direction()),
		b = dot(r.direction(), CA),
		c = dot(CA, CA) - radius * radius,
		discriminant = b * b - a*c;
	if (discriminant < 0)
	{
		return false;
	}
	else
	{
		float solution = (-b - sqrt(discriminant)) / a;
		if (solution > t_min && solution < t_max)
		{
			rec.t = solution;
			rec.p = r.point_at_t(solution);
			rec.normal = (rec.p - center) / radius;
			rec.material_ptr = material;
			return true;
		}
		solution = (-b + sqrt(discriminant)) / a;
		if (solution > t_min && solution < t_max)
		{
			rec.t = solution;
			rec.p = r.point_at_t(solution);
			rec.normal = (rec.p - center) / radius;
			rec.material_ptr = material;
			return true;
		}
	}
}

bool Hitable_list::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
	hit_record tmp_record;
	bool hit_anything = false;
	double closet_t_so_far = t_max;
	for (auto object : l)
	{
		if (object->hit(r, t_min, closet_t_so_far, tmp_record))
		{
			hit_anything = true;
			closet_t_so_far = tmp_record.t;
			rec = tmp_record;
		}
	}
	return hit_anything;
}