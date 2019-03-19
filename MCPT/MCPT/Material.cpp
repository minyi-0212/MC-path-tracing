#include "Material.h"

vec3 random_in_unit_sphere()
{
	vec3 p;
	do {
		p = vec3(2 * random_float_0_1(), 2 * random_float_0_1(), 2 * random_float_0_1()) - vec3(1);
	} while (length(p) >= 1.0);
	return p;
}

bool Lambertian::scatter(const Ray& r_in, const hit_record& rec,
	scatter_record& scatter_rec)
{
	vec3 s = rec.p + rec.normal + random_in_unit_sphere();
	//scattered = Ray(rec.p, normalize(s-rec.p));
	//attenuation = albedo;
	//pdf = dot(rec.normal, scattered.direction()) / M_PI; // cosθ
	scatter_rec.is_specular = false;
	scatter_rec.albedo = albedo;
	scatter_rec.pdf_ptr = new PDF_cos(rec.normal);
	return true;
}

float Lambertian::scattering_pdf(const Ray& r_in, const hit_record& rec,
	const Ray& scattered)
{
	float cosine = dot(rec.normal, normalize(scattered.direction()));
	if (cosine < 0)
		cosine = 0;
	return cosine / M_PI;
}

vec3 reflect(const vec3& v, const vec3& n)
{
	return v - 2 * dot(v, n)*n;
}

bool Metal::scatter(const Ray& r_in, const hit_record& hit_rec, scatter_record& scatter_rec) const
{
	vec3 ref = reflect(normalize(r_in.direction()), hit_rec.normal);
	scatter_rec.is_specular = true;
	scatter_rec.albedo = albedo;
	scatter_rec.specular_ray = Ray(hit_rec.p, ref + fuzzier * random_in_unit_sphere());
	scatter_rec.pdf_ptr = nullptr;
	return true;
}

bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted)
{
	//n sin(theta) = n’ sin(theta’)
	vec3 L = normalize(v);
	float cos1 = dot(-L, n);
	float discriminant = 1.0f - ni_over_nt * ni_over_nt*(1 - cos1 * cos1);
	if (discriminant > 0) {
		refracted = ni_over_nt * (L + n * cos1) - n * sqrt(discriminant);
		return true;
	}
	else
		return false;
}

//发生折射的概率，schlick：近似地计算出不同入射角旳菲涅耳反射比
float schlick(float cosine, float Ni)
{
	float r0 = (1 - Ni) / (1 + Ni);
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
		ni_over_nt = Ni;
		//cosine = Ni * dot(r_in.direction(), rec.normal) / r_in.direction().length();
		cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
		cosine = sqrt(1 - Ni * Ni*(1 - cosine * cosine));
	}
	else {
		outward_normal = rec.normal;
		ni_over_nt = 1.0f / Ni;
		cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
	}

	if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
		reflect_prob = schlick(cosine, Ni);
	else
		reflect_prob = 1.0;

	// choose
	if (random_float_0_1() < reflect_prob)
		scattered = Ray(rec.p, reflected);
	else
		scattered = Ray(rec.p, refracted);
	return true;
}