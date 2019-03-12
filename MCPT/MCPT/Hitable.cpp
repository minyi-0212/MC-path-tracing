#include "Hitable.h"
#include <iostream>

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

bool Sphere::bounding_box(float t0, float t1, AABB& box) const
{
	box = AABB(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
	return true;
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

bool Hitable_list::bounding_box(float t0, float t1, AABB& box) const 
{
	if (l.size() == 0) 
		return false;
	AABB temp_box;
	bool first_true = l.front()->bounding_box(t0, t1, temp_box);
	if (!first_true)
		return false;
	else
		box = temp_box;
	for (auto object : l)
	{
		if (object->bounding_box(t0, t1, temp_box)) {
			box = merge_box(box, temp_box);
		}
		else
			return false;
	}
	return true;
}


bool Bvh::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const 
{
	if (box.hit(r, t_min, t_max)) {
		hit_record left_rec, right_rec;
		bool hit_left = left->hit(r, t_min, t_max, left_rec);
		bool hit_right = right->hit(r, t_min, t_max, right_rec);
		if (hit_left && hit_right) {
			if (left_rec.t < right_rec.t)
				rec = left_rec;
			else
				rec = right_rec;
			return true;
		}
		else if (hit_left) {
			rec = left_rec;
			return true;
		}
		else if (hit_right) {
			rec = right_rec;
			return true;
		}
		else
			return false;
	}
	else return false;
}

bool Bvh::bounding_box(float t0, float t1, AABB& b) const 
{
	b = box;
	return true;
}

int box_x_compare(const void * a, const void * b) {
	AABB box_left, box_right;
	Hitable *ah = *(Hitable**)a;
	Hitable *bh = *(Hitable**)b;
	if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
		std::cerr << "no bounding box in Bvh constructor[compare x]\n";
	if (box_left.min()[0] - box_right.min()[0] < 0.0)
		return -1;
	else
		return 1;
}
int box_y_compare(const void * a, const void * b)
{
	AABB box_left, box_right;
	Hitable *ah = *(Hitable**)a;
	Hitable *bh = *(Hitable**)b;
	if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
		std::cerr << "no bounding box in bvh_node constructor[compare y]\n";
	if (box_left.min()[1] - box_right.min()[1] < 0.0)
		return -1;
	else
		return 1;
}
int box_z_compare(const void * a, const void * b)
{
	AABB box_left, box_right;
	Hitable *ah = *(Hitable**)a;
	Hitable *bh = *(Hitable**)b;
	if (!ah->bounding_box(0, 0, box_left) || !bh->bounding_box(0, 0, box_right))
		std::cout << "no bounding box in Bvh constructor[compare z]\n";
	if (box_left.min()[2] - box_right.min()[2] < 0.0)
		return -1;
	else
		return 1;
}

Bvh::Bvh(Hitable **l, int n, float time0, float time1) {
	int axis = int(3 * random_float_0_1());
	if (axis == 0)
		qsort(l, n, sizeof(Hitable *), box_x_compare);
	else if (axis == 1)
		qsort(l, n, sizeof(Hitable *), box_y_compare);
	else
		qsort(l, n, sizeof(Hitable *), box_z_compare);
	if (n == 1) {
		left = right = l[0];
	}
	else if (n == 2) {
		left = l[0];
		right = l[1];
	}
	else {
		left = new Bvh(l, n / 2, time0, time1);
		right = new Bvh(l + n / 2, n - n / 2, time0, time1);
	}
	AABB box_left, box_right;
	if (!left->bounding_box(time0, time1, box_left) || !right->bounding_box(time0, time1, box_right))
		std::cout << "no bounding box in Bvh constructor\n";
	box = merge_box(box_left, box_right);
}