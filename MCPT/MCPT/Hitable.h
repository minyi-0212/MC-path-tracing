#pragma once
#include <list>
#include "Ray.h"
#include "AABB.h"
using std::list;

class Material;
struct hit_record
{
	float t;
	vec3 p, normal;
	Material* material_ptr;
};

// object
class Hitable
{
public:
	virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(float t0, float t1, AABB& box) const = 0;
};

// sphere
class Sphere : public Hitable
{
public:
	Sphere() {}
	Sphere(vec3 center, float radius, Material* const material):center(center), radius(radius), material(material){}
	virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, AABB& box) const;
private:
	vec3 center;
	float radius;
	Material* material;
};

// hitable_list: list of object
class Hitable_list : public Hitable
{
public:
	Hitable_list() {}
	Hitable_list(list<Hitable*>& l) :l(l) {}
	virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, AABB& box) const;

private:
	list<Hitable*> l;
};

// bvh -- kdtree
class Bvh : public Hitable {
public:
	Bvh() {}
	Bvh(Hitable **l, int n, float time0, float time1);
	virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, AABB& box) const;
private:
	Hitable *left;
	Hitable *right;
	AABB box;
};