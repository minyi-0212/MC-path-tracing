#include "Material.h"

vec3 random_in_unit_sphere()
{
	vec3 p;
	do {
		p = vec3(2 * random_float_0_1(), 2 * random_float_0_1(), 2 * random_float_0_1()) - vec3(1);
	} while (length(p) >= 1.0);
	return p;
}

bool Lambertian::scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered) const
{
	vec3 s = rec.p + rec.normal + random_in_unit_sphere();
	attenuation = albedo;
	scattered = Ray(rec.p, s - rec.p);
	return true;
}

vec3 reflect(const vec3& v, const vec3& n)
{
	return v - 2 * dot(v, n)*n;
}

bool Metal::scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered) const
{
	vec3 ref = reflect(normalize(r_in.direction()), rec.normal);
	attenuation = albedo;
	scattered = Ray(rec.p, ref + fuzzier * random_in_unit_sphere());
	return (dot(scattered.direction(), rec.normal) >0);
}

bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted)
{
	//n sin(theta) = n’ sin(theta’)
	vec3 L = normalize(v);
	float cos1 = dot(-L, n);
	float discriminant = 1.0 - ni_over_nt * ni_over_nt*(1 - cos1 * cos1);
	if (discriminant > 0) {
		refracted = ni_over_nt * (L + n * cos1) - n * sqrt(discriminant);
		return true;
	}
	else
		return false;
}

//发生折射的概率，schlick：近似地计算出不同入射角旳菲涅耳反射比
float schlick(float cosine, float ref_idx)
{
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0)*pow((1 - cosine), 5);
}

bool Dielectric::scatter(const Ray& r_in, const hit_record& rec, vec3& attenuation, Ray& scattered) const {
	// reflection
	vec3 reflected = reflect(r_in.direction(), rec.normal);

	// refraction
	vec3 outward_normal;
	float ni_over_nt;
	attenuation = vec3(1.0, 1.0, 1.0);
	vec3 refracted;
	float reflect_prob;
	float cosine;
	if (dot(r_in.direction(), rec.normal) > 0) {
		outward_normal = -rec.normal;
		ni_over_nt = ref_idx;
		//cosine = ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
		cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
		cosine = sqrt(1 - ref_idx * ref_idx*(1 - cosine * cosine));
	}
	else {
		outward_normal = rec.normal;
		ni_over_nt = 1.0 / ref_idx;
		cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
	}

	if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
		reflect_prob = schlick(cosine, ref_idx);
	else
		reflect_prob = 1.0;

	// choose
	if (random_float_0_1() < reflect_prob)
		scattered = Ray(rec.p, reflected);
	else
		scattered = Ray(rec.p, refracted);
	return true;
}