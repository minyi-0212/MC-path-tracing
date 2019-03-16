#pragma once
#include "Ray.h"
#include "Hitable.h"
#include <vector>

using std::vector;

vec3 random_in_unit_sphere();

class Material
{
public:
	virtual bool scatter(const Ray& r_in, const hit_record& rec, 
		vec3& attenuation, Ray& scattered) const = 0; // attenuation:less scattered:direction
	virtual vec3 emitted() { return vec3(0); };
};

class Lambertian :public Material
{
public:
	Lambertian(const vec3& a):albedo(a){}
	virtual bool scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered) const;
private:
	vec3 albedo;
};

class Metal :public Material
{
public:
	Metal(const vec3& a, float f) :albedo(a)
	{
		fuzzier = f > 1 ? 1 : f;
	}
	virtual bool scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered) const;
private:
	vec3 albedo;
	float fuzzier;
};

class Dielectric : public Material {
public:
	Dielectric(float Ni) : Ni(Ni) {}
	virtual bool scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered) const;

private:
	float Ni; // refractive index
};

class Diffuse_light : public Material
{
public:
	Diffuse_light(const vec3& v) :Le(v) {};
	virtual bool scatter(const Ray& r_in, const hit_record& rec,
		vec3& attenuation, Ray& scattered) const 
	{
		return false;
	};
	virtual vec3 emitted() { return Le; };

private:
	vec3 Le;
};