#include "Hitable.h"
#include <algorithm>

#define min3(a,b,c) __min(a,__min(b,c))
#define max3(a,b,c) __max(a,__max(b,c))

bool Sphere::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
	/// A:r.origin() B:r.direction() C:center
	/// t*t*dot(B, B) + 2 * t*dot(B, A - C) + dot(A - C, A - C) - R * R = 0;
	vec3 CA = r.origin() - center;
	float a = dot(r.direction(), r.direction()),
		b = dot(r.direction(), CA),
		c = dot(CA, CA) - radius * radius,
		discriminant = b * b - a * c;
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

float Sphere::pdf_value(const vec3& origin, const vec3& v)  const
{
	hit_record hit_rec;
	if (!this->hit(Ray(origin, v), RAY_MIN_T, INT_MAX, hit_rec) || (radius / length(center - origin)) > 1)
		return 0;
	else
		//if (this->hit(Ray(origin, v), RAY_MIN_T, INT_MAX, hit_rec))
	{
		float cos_theta_max = sqrt(1 - radius * radius / (length(center - origin)*length(center - origin))),
			solid_angle = 2 * M_PI*(1 - cos_theta_max);
		if (isnan(cos_theta_max))
			cout << "sphere light pdf value is nan " << endl;
		return  1 / solid_angle;
	}
	/*else
		return 0;*/
}

inline vec3 random_to_sphere(float radius, float distance_square)
{
	float cos_theta = radius / distance_square;
	if (cos_theta > 1) 
		cos_theta = 1;
	float r1 = random_float_0_1(),
		r2 = random_float_0_1(),
		z = 1 + r2 * (sqrt(1 - cos_theta * cos_theta) - 1),
		phi = 2 * M_PI * r1;
	return vec3(cos(phi)*sqrt(1 - z * z), sin(phi)*sqrt(1 - z * z), z);
}

vec3 Sphere::random(const vec3& origin) const
{
	vec3 direction = center - origin;
	float distance = length(direction);
	OrthonormalBases frame;
	frame.build_frame(direction);
	vec3 tmp(random_to_sphere(radius, distance));
	if (isnan(tmp[0]))
		cout << "sphere: nan - " << tmp << " " << radius << " " << distance << endl;
	return frame.local(tmp);
}

bool Sphere::in_obj(const vec3& p) const
{
	if (fabs(length(center - p)) <= radius)
		return true;
	else
		return false;
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

float Hitable_list::pdf_value(const vec3 & o, const vec3 & v) const
{
	float weight = 1.0 / l.size();
	float sum = 0;
	for (auto object : l)
		sum += weight * object->pdf_value(o, v);
	return sum;
}

vec3 Hitable_list::random(const vec3 & o) const
{
	int index = floor(random_float_0_1() * l.size());
	list<Hitable*>::iterator iter = l.begin();
	if (index != 0 && index != l.size())
		advance(iter, index);
	return (*iter)->random(o);
}

bool Hitable_list::in_obj(const vec3& p) const
{
	for (auto obj : l)
	{
		if (obj->in_obj(p))
			return true;
	}
	return false;
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

bool Bvh::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
	box.hit(r, t_min, t_max);
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
	else
		return false;
}

bool Bvh::bounding_box(float t0, float t1, AABB& b) const
{
	b = box;
	return true;
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

float RectXY::pdf_value(const vec3& origin, const vec3& v) const
{
	hit_record rec;
	if (this->hit(Ray(origin, v), RAY_MIN_T, FLT_MAX, rec)) {
		float area = (x1 - x0)*(y1 - y0);
		float distance_squared = rec.t * rec.t * length(v)* length(v);
		float cosine = fabs(dot(v, rec.normal) / length(v));
		return  distance_squared / (cosine * area);
	}
	else
		return 0;
}

vec3 RectXY::random(const vec3& origin) const
{
	vec3 random_point = vec3(x0 + random_float_0_1()*(x1 - x0), y0 + random_float_0_1()*(y1 - y0), k);
	return random_point - origin;
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

float RectXZ::pdf_value(const vec3& origin, const vec3& v) const
{
	hit_record rec;
	if (this->hit(Ray(origin, v), RAY_MIN_T, FLT_MAX, rec)) {
		float area = (x1 - x0)*(z1 - z0);
		float distance_squared = rec.t * rec.t * length(v)* length(v);
		float cosine = fabs(dot(v, rec.normal) / length(v));
		return  distance_squared / (cosine * area);
	}
	else
		return 0;
}

vec3 RectXZ::random(const vec3& origin) const
{
	vec3 random_point = vec3(x0 + random_float_0_1()*(x1 - x0), k, z0 + random_float_0_1()*(z1 - z0));
	return random_point - origin;
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

float RectYZ::pdf_value(const vec3& origin, const vec3& v) const
{
	hit_record rec;
	if (this->hit(Ray(origin, v), RAY_MIN_T, FLT_MAX, rec)) {
		float area = (y1 - y0)*(z1 - z0);
		float distance_squared = rec.t * rec.t * length(v)* length(v);
		float cosine = fabs(dot(v, rec.normal) / length(v));
		return  distance_squared / (cosine * area);
	}
	else
		return 0;
}

vec3 RectYZ::random(const vec3& origin) const
{
	vec3 random_point = vec3(k, y0 + random_float_0_1()*(y1 - y0), z0 + random_float_0_1()*(z1 - z0));
	return random_point - origin;
}


bool Triangle::hit(const Ray& r, float t_min, float t_max, hit_record& rec) const
{
	vec3 q = cross(r.direction(), e2);
	float alpha = dot(e1, q);
	if (abs(alpha) == 0)
		return false;
	float factor = 1.0 / alpha;
	vec3 s = r.origin() - v0;
	float u = dot(s, q) * factor;
	if (u < 0. || u > 1.)
		return false;
	vec3 rr = cross(s, e1);
	float v = factor * dot(r.direction(), rr);
	if (v < 0. || v + u >1.)
		return false;
	float t = factor * dot(e2, rr);
	if (t > t_min && t < t_max)
	{
		rec.t = t;
		rec.p = r.point_at_t(t);
		rec.material_ptr = material;
		rec.normal = normal;  // simple, need to compute with the normal in vertex
		return true;
	}
	else
		return false;
}

bool Triangle::bounding_box(float t0, float t1, AABB& box) const
{
	vec3 low(min3(v0[0], v1[0], v2[0]),
		min3(v0[1], v1[1], v2[1]),
		min3(v0[2], v1[2], v2[2])),
		high(max3(v0[0], v1[0], v2[0]),
			max3(v0[1], v1[1], v2[1]),
			max3(v0[2], v1[2], v2[2]));

	box = AABB(low, high);
	return true;
}