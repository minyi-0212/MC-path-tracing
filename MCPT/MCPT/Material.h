#pragma once
#include "Ray.h"
#include "PDF.h"
#include <vector>
#include <memory.h>

using std::vector;

struct scatter_record
{
	int status;  // 0£ºlambertian 1: reflect 2:refract
	Ray specular_ray; // direction
	vec3 albedo; // attenuation
	std::shared_ptr<PDF> pdf_ptr;
};

class Material
{
public:
	virtual bool scatter(const Ray& r_in, const hit_record& rec,
		scatter_record& scatter_rec)
	{
		return false;
	}
	// directional distribution
	virtual float scattering_pdf(const Ray& r_in, const hit_record& rec,
		const Ray& scattered)
	{
		return false;
	}
	virtual float scattering_pdf_value_for_blinn_phone(const Ray& r_in, const hit_record& rec, const Ray& scattered)
	{
		cout << "virtual construction: blinn phone" << endl;
		return 0;
	}
	virtual vec3 emitted(const Ray& r_in, const hit_record& rec) const { return vec3(0); }
};

class Lambertian :public Material
{
public:
	Lambertian(const vec3& a) :_albedo(a) {}
	virtual bool scatter(const Ray& r_in, const hit_record& rec, scatter_record& scatter_rec);
	virtual float scattering_pdf(const Ray& r_in, const hit_record& rec, const Ray& scattered);
private:
	vec3 _albedo;
};

class Metal :public Material
{
public:
	Metal(const vec3& a, float f) :_albedo(a)
	{
		_fuzzier = f > 1 ? 1 : f;
	}
	virtual bool scatter(const Ray& r_in, const hit_record& rec, scatter_record& scatter_rec) const;
private:
	vec3 _albedo;
	float _fuzzier;
};

class Dielectric : public Material
{
public:
	Dielectric(float _Ni) : _Ni(_Ni) {}
	virtual bool scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered) const;

private:
	float _Ni; // refractive index
};

class Light : public Material
{
public:
	Light(const vec3& v) :_Le(v) {};
	virtual bool scatter(const Ray& r_in, const hit_record& rec,
		vec3& attenuation, Ray& scattered) const
	{
		return false;
	};

	virtual vec3 emitted(const Ray& r_in, const hit_record& rec) const;

private:
	vec3 _Le;
};

struct mtl_param
{
	std::string name;
	float Ns, // shiness
		Ni; // refractive index
	int illum;
	glm::vec3 Ka, Kd, Ks, Tf; // ambient diffuse specular transmission filter

};

class MTL : public Material
{
public:
	MTL(){};
	virtual bool scatter(const Ray& r_in, const hit_record& rec, scatter_record& scatter_rec);
	virtual float scattering_pdf(const Ray& r_in, const hit_record& rec, const Ray& scattered);
	virtual float scattering_pdf_value_for_blinn_phone(const Ray& r_in, const hit_record& rec, const Ray& scattered);

	mtl_param para;
};