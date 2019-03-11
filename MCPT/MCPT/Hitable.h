#pragma once
#include <list>
#include "Ray.h"
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
};

class Sphere : public Hitable
{
public:
	Sphere() {}
	Sphere(vec3 center, float radius, Material* const material):center(center), radius(radius), material(material){}
	virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
private:
	vec3 center;
	float radius;
	Material* material;
};

//object list
class Hitable_list : public Hitable
{
public:
	Hitable_list() {}
	Hitable_list(list<Hitable*>& l) :l(l) {}
	virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;

private:
	list<Hitable*> l;
};