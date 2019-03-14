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

int box_x_compare(const Hitable* a, const Hitable* b) {
	AABB box_left, box_right;
	if (!a->bounding_box(0, 0, box_left) || !b->bounding_box(0, 0, box_right))
		std::cout << "no bounding box in Bvh constructor[compare x]\n";
	if (box_left.min()[0] - box_right.min()[0] < 0.0)
		return -1;
	else
		return 1;
}
int box_y_compare(const Hitable* a, const Hitable* b)
{
	AABB box_left, box_right;
	if (!a->bounding_box(0, 0, box_left) || !b->bounding_box(0, 0, box_right))
		std::cout << "no bounding box in bvh_node constructor[compare y]\n";
	if (box_left.min()[1] - box_right.min()[1] < 0.0)
		return -1;
	else
		return 1;
}
int box_z_compare(const Hitable* a, const Hitable* b)
{
	AABB box_left, box_right;
	if (!a->bounding_box(0, 0, box_left) || !b->bounding_box(0, 0, box_right))
		std::cout << "no bounding box in Bvh constructor[compare z]\n";
	if (box_left.min()[2] - box_right.min()[2] < 0.0)
		return -1;
	else
		return 1;
}

Bvh::Bvh(list<Hitable*>& l, float time0, float time1) {
	int axis = int(3 * random_float_0_1()), 
		n = l.size();
	if (axis == 0)
		l.sort(box_x_compare);
	else if (axis == 1)
		l.sort(box_y_compare);
	else
		l.sort(box_z_compare);

	if (n == 1) {
		left = right = l.front();
	}
	else if (n == 2) {
		left = l.front();
		right = l.back();
	}
	else {
		list<Hitable*> l_new;
		list<Hitable*>::iterator first_iter = l_new.begin(),
			n1 = l.begin(), n2 = l.end();
		advance(n1, n / 2);
		l_new.splice(first_iter, l, n1, n2);
		left = new Bvh(l, time0, time1);
		right = new Bvh(l_new, time0, time1);
	}
	AABB box_left, box_right;
	if (!left->bounding_box(time0, time1, box_left) || !right->bounding_box(time0, time1, box_right))
		std::cout << "no bounding box in Bvh constructor\n";
	box = merge_box(box_left, box_right);
}

bool RectXY::hit(const Ray& r, float t0, float t1, hit_record& rec) const {
	float t = (k - r.origin()[2]) / r.direction()[2];
	if (t < t0 || t > t1)
		return false;
	float x = r.origin()[0] + t * r.direction()[0];
	float y = r.origin()[1] + t * r.direction()[1];
	if (x < x0 || x > x1 || y < y0 || y > y1)
		return false;
	rec.t = t;
	rec.p = r.point_at_t(t);
	rec.material_ptr = material;
	rec.normal = vec3(0, 0, 1);
	return true;
}


bool RectXZ::hit(const Ray& r, float t0, float t1, hit_record& rec) const {
	float t = (k - r.origin()[1]) / r.direction()[1];
	if (t < t0 || t > t1)
		return false;
	float x = r.origin()[0] + t * r.direction()[0];
	float z = r.origin()[2] + t * r.direction()[2];
	if (x < x0 || x > x1 || z < z0 || z > z1)
		return false;
	rec.t = t;
	rec.p = r.point_at_t(t);
	rec.material_ptr = material;
	rec.normal = vec3(0, 1, 0);
	return true;
}

bool RectYZ::hit(const Ray& r, float t0, float t1, hit_record& rec) const {
	float t = (k - r.origin()[0]) / r.direction()[0];
	if (t < t0 || t > t1)
		return false;
	float y = r.origin()[1] + t * r.direction()[1];
	float z = r.origin()[2] + t * r.direction()[2];
	if (y < y0 || y > y1 || z < z0 || z > z1)
		return false;
	rec.t = t;
	rec.p = r.point_at_t(t);
	rec.material_ptr = material;
	rec.normal = vec3(1, 0, 0);
	return true;
}