#include "Material.h"

vec3 random_in_unit_sphere()
{
	vec3 p;
	do {
		p = vec3(2 * random_float_0_1(), 2 * random_float_0_1(), 2 * random_float_0_1()) - vec3(1);
	} while (length(p) >= 1.0);
	return p;
}

bool Lambertian::scatter(const Ray& r_in, const hit_record& rec, scatter_record& scatter_rec) const
{
	scatter_rec.status = 0;
	scatter_rec.albedo = _albedo;
	scatter_rec.pdf_ptr = std::make_shared<PDF_cos>(rec.normal);
	if (length(rec.normal) == 0)
	{
		vec3 tmp(rec.normal);
		cout << "Lambertian normal length is 0: " << tmp << endl;
	}
	return true;
}

float Lambertian::scattering_pdf(const Ray& r_in, const hit_record& rec, const Ray& scattered) const
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
	scatter_rec.status = 2;
	scatter_rec.albedo = _albedo;
	scatter_rec.specular_ray = Ray(hit_rec.p, ref + _fuzzier * random_in_unit_sphere());
	scatter_rec.pdf_ptr = nullptr;
	return true;
}


bool refract(const vec3& v, const vec3& n, float ni_over_nt, vec3& refracted)
{
	//n sin(theta) = n¡¯ sin(theta¡¯)
	vec3 L = normalize(v);
	float cos1 = dot(-L, n);
	float discriminant = 1.0f - ni_over_nt * ni_over_nt*(1 - cos1 * cos1);
	if (discriminant > 0) {
		refracted = ni_over_nt * (L + n * cos1) - n * sqrt(discriminant);
		if (length(refracted) == 0)
		{
			cout << "debug:refract direction is (0,0,0)" << endl;
		}
		return true;
	}
	else
		return false;
}

float schlick(float cosine, float _Ni)
{
	float r0 = (1 - _Ni) / (1 + _Ni);
	r0 = r0 * r0;
	return r0 + (1 - r0)*pow((1 - cosine), 5);
}

bool Dielectric::scatter(const Ray& r_in, const hit_record& hit_rec, scatter_record& scatter_rec) const
{
	// reflection
	vec3 reflected = reflect(r_in.direction(), hit_rec.normal);

	// refraction
	vec3 outward_normal;
	float ni_over_nt;
	scatter_rec.status = 2;
	scatter_rec.albedo = vec3(1.0, 1.0, 1.0);
	vec3 refracted;
	float reflect_prob;
	float cosine;
	if (dot(r_in.direction(), hit_rec.normal) > 0) {
		outward_normal = -hit_rec.normal;
		ni_over_nt = _Ni;
		//cosine = Ni * dot(r_in.direction(), rec.normal) / r_in.direction().length();
		cosine = dot(r_in.direction(), hit_rec.normal);
		cosine = sqrt(1 - _Ni * _Ni*(1 - cosine * cosine));
	}
	else {
		outward_normal = hit_rec.normal;
		ni_over_nt = 1.0f / _Ni;
		cosine = -dot(r_in.direction(), hit_rec.normal);
	}

	if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
		reflect_prob = schlick(cosine, _Ni);
	else
		reflect_prob = 1.0;

	// choose
	if (random_float_0_1() < reflect_prob)
		scatter_rec.specular_ray = Ray(hit_rec.p, reflected);
	else
		scatter_rec.specular_ray = Ray(hit_rec.p, refracted);
	return true;
}

vec3 Light::emitted(const Ray& r_in, const hit_record& rec) const
{
	/*if (dot(r_in.direction(), rec.normal) > 0)
		return Le;
	else
		return vec3(0);*/
	return _Le;
};


bool MTL::scatter(const Ray& r_in, const hit_record& hit_rec, scatter_record& scatter_rec) const
{
	float has_diffuse = dot(para.Kd, vec3(1.0)),
		has_specular = dot(para.Ks, vec3(1.0));
	if (has_diffuse + has_specular <= 0)
		return false;

	// refraction
	if (para.illum && para.Ni != 1.0)
	{
		// reflection
		vec3 reflected = reflect(r_in.direction(), hit_rec.normal);

		// refraction
		vec3 outward_normal;
		float ni_over_nt;
		vec3 refracted;
		float reflect_prob, cosine;
		if (dot(r_in.direction(), hit_rec.normal) > 0) {
			outward_normal = -hit_rec.normal;
			ni_over_nt = para.Ni;
			cosine = para.Ni*dot(r_in.direction(), hit_rec.normal);
		}
		else {
			outward_normal = hit_rec.normal;
			ni_over_nt = 1.0f / para.Ni;
			cosine = -dot(r_in.direction(), hit_rec.normal);
		}
		if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
		{
			reflect_prob = schlick(cosine, para.Ni);
		}
		else
			reflect_prob = 1.1;

		// choose
		if (random_float_0_1() < reflect_prob)
		{
			scatter_rec.status = 2;
			scatter_rec.albedo = para.Kd;
			scatter_rec.specular_ray = Ray(hit_rec.p, reflected /*+ vec3(0.5 / (para.Ns + 1)) * random_in_unit_sphere()*/);
			//scatter_rec.pdf_ptr = std::make_shared<PDF_cos_n>(hit_rec.normal, para.Ni);
			if (length(reflected) == 0)
			{
				cout << "MTL Ni:reflected length is 0: " << reflected << endl;
			}
		}
		else
		{
			scatter_rec.status = 2;
			scatter_rec.albedo = vec3(1.0);
			//scatter_rec.albedo = vec3(para.Kd[0]);
			scatter_rec.specular_ray = Ray(hit_rec.p, refracted);
			//scatter_rec.pdf_ptr = std::make_shared<PDF_cos_n>(refracted, para.Ni);
			if (length(refracted) == 0)
			{
				cout << "MTL Ni:refracted length is 0: " << refracted << endl;
			}
		}
		return true;
	}

	// reflection
	if (has_specular
		&& (random_float_0_1() > (has_diffuse / has_specular))
		//&& (random_float_0_1() > 0.5)
		&& (dot(-r_in.direction(), hit_rec.normal) >= 0))
	{
		vec3 reflected = reflect(normalize(r_in.direction()), normalize(hit_rec.normal));
		scatter_rec.status = 2;
		scatter_rec.albedo = para.Ks;
		scatter_rec.specular_ray = Ray(hit_rec.p, reflected /*+ vec3(0.1*(1 - para.Ns / 1000)) * random_in_unit_sphere()*/);
		scatter_rec.pdf_ptr = std::make_shared<PDF_cos_n>(hit_rec.normal, para.Ns);
		return true;
	}
	// lambertian
	else
	{
		scatter_rec.status = 0;
		scatter_rec.albedo = para.Kd;
		scatter_rec.pdf_ptr = std::make_shared<PDF_cos>(hit_rec.normal);
		return true;
	}
}

float MTL::scattering_pdf(const Ray& r_in, const hit_record& rec, const Ray& scattered) const
{
	float cosine = dot(rec.normal, scattered.direction());
	if (cosine < 0)
		cosine = 0;
	return cosine / M_PI;
}

float MTL::scattering_pdf_value_for_blinn_phone(const Ray& r_in, const hit_record& rec, const Ray& scattered) const
{
	// importance sampling : scater_pdf/pdf = 4*((n+2)/(n+1)*(wo¡¤wh))
	/*vec3 tmp = normalize(scattered.direction());
	tmp += normalize(r_in.direction());
	tmp /= 2;
	float result = (4 + 4.0 / (para.Ns + 1))*dot(normalize(scattered.direction()), normalize(tmp));
	return result;*/
	if (dot(-r_in.direction(), rec.normal) <= 0 || dot(scattered.direction(), rec.normal) <= 0)
		return 0;
	vec3 H = -r_in.direction();
	H += scattered.direction();
	H = normalize(H);
	float tt = dot(normalize(rec.normal), H);
	return pow(tt, para.Ni);
}