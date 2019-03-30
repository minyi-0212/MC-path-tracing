#pragma once
#include "Ray.h"
#include "AABB.h"
#include "OrthonormalBases.h"
#include <list>
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
	virtual float pdf_value(const vec3& origin, const vec3& v)  const { return 0; }
	virtual vec3 random(const vec3& origin) const
	{
		std::cout << "in virtual construct Hitable::random" << std::endl;
		return vec3(0);
	}
	virtual bool in_obj(const vec3& p) const { return false; }
};

// sphere
class Sphere : public Hitable
{
public:
	Sphere() {}
	Sphere(vec3 center, float radius, Material* const material) :center(center), radius(radius), material(material) {}
	virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, AABB& box) const;
	virtual float pdf_value(const vec3& origin, const vec3& v)  const;
	virtual vec3 random(const vec3& origin) const;
	virtual bool in_obj(const vec3& p) const;
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
	virtual float pdf_value(const vec3& o, const vec3& v) const;
	virtual vec3 random(const vec3& o) const;
	virtual bool in_obj(const vec3& p) const;

private:
	mutable list<Hitable*> l;
};

class Bvh : public Hitable {
public:
	Bvh() {}
	Bvh(list<Hitable*>& l, float time0, float time1);
	virtual bool hit(const Ray& r, float tmin, float tmax, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, AABB& box) const;
private:
	Hitable *left;
	Hitable *right;
	AABB box;
};

class RectXY : public Hitable {
public:
	RectXY() {}
	RectXY(float _x0, float _x1, float _y0, float _y1, float _k, Material *mat) :
		x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), material(mat) {};
	virtual bool hit(const Ray& r, float t0, float t1, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, AABB& box) const {
		box = AABB(vec3(x0, y0, k - 0.0001), vec3(x1, y1, k + 0.0001));
		return true;
	}
	virtual float pdf_value(const vec3& origin, const vec3& v) const;
	virtual vec3 random(const vec3& origin) const;
	Material  *material;
	float x0, x1, y0, y1, k;
};

class RectXZ : public Hitable {
public:
	RectXZ() {}
	RectXZ(float _x0, float _x1, float _z0, float _z1, float _k, Material *mat) :
		x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), material(mat) {};
	virtual bool hit(const Ray& r, float t0, float t1, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, AABB& box) const {
		box = AABB(vec3(x0, k - 0.0001, z0), vec3(x1, k + 0.0001, z1));
		return true;
	}
	virtual float pdf_value(const vec3& origin, const vec3& v) const;
	virtual vec3 random(const vec3& origin) const;
private:
	Material  *material;
	float x0, x1, z0, z1, k;
};

class RectYZ : public Hitable {
public:
	RectYZ() {}
	RectYZ(float _y0, float _y1, float _z0, float _z1, float _k, Material *mat) :
		y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), material(mat) {};
	virtual bool hit(const Ray& r, float t0, float t1, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, AABB& box) const {
		box = AABB(vec3(k, y0, z0), vec3(k, y1, z1));
		return true;
	}
	virtual float pdf_value(const vec3& origin, const vec3& v) const;
	virtual vec3 random(const vec3& origin) const;
	Material  *material;
	float y0, y1, z0, z1, k;
};

class Triangle : public Hitable
{
public:
	Triangle(const vec3& v0, const vec3 & v1, const vec3 & v2, Material *mat) :
		v0(v0), v1(v1), v2(v2), e1(v1 - v0), e2(v2 - v0),
		normal(normalize(cross(v1 - v0, v2 - v0))), material(mat) {}
	virtual bool hit(const Ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(float t0, float t1, AABB& box) const;

private:
	vec3 v0, v1, v2, e1, e2, normal;
	Material  *material;
};