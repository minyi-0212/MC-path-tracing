#include <iostream>
#include <fstream>
//#include <glut.h>
#include <gtc/matrix_transform.hpp>
#include <math.h>
#include "Material.h"
#include "Camera.h"
#include "obj_loader.h"
#include "Performance.h"
#include "PDF.h"

using std::cout;
using std::endl;

vec3 color(const Ray& r, Hitable& object_list, Hitable& light, const int depth)
{
	// the color for hit
	hit_record hit_rec;
	if (object_list.hit(r, 0.001, INT_MAX, hit_rec))
	{
		float pdf_value;
		scatter_record scatter_rec;
		if (depth < 10 && hit_rec.material_ptr->scatter(r, hit_rec, scatter_rec))
		{
			//cout << "depth" << depth << endl;
			if (scatter_rec.is_specular)
			{
				return scatter_rec.albedo*color(scatter_rec.specular_ray, object_list, light, depth + 1);
			}
			else
			{
				/*cout << hit_rec.normal[0] << " "
					<< hit_rec.normal[1] << " "
					<< hit_rec.normal[2] << " " << endl;*/
				PDF_to_light pdf_light(light, hit_rec.p);
				PDF_mix pdf(&pdf_light, scatter_rec.pdf_ptr);
				Ray scattered;
				scattered = Ray(hit_rec.p, pdf.generate_random_d());
				pdf_value = pdf.value(scattered.direction());
				//cout << dot(hit_rec.normal, scattered.direction()) << " " << pdf_value << endl;
				return hit_rec.material_ptr->emitted() + scatter_rec.albedo * hit_rec.material_ptr->scattering_pdf(r, hit_rec, scattered) *
					color(scattered, object_list, light, depth + 1) / pdf_value;
			}
		}
		else
			return hit_rec.material_ptr->emitted();
	}
	else
		return vec3(0);
	/*vec3 unit_direction = r.direction();
	float t = unit_direction[0] == 0 && unit_direction[1] == 0 && unit_direction[2] == 0 ?
	0.5 : 0.5*(normalize(unit_direction)[1] + 1.0);
	return vec3(1.0 - t, 1.0 - t, 1.0 - t) + vec3(t*0.5, t*0.7, t*1.0);*/
}

void random_scene(list<Hitable*>& list)
{
	int n = 0;
	//list.push_back(new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(vec3(0.5))));
	for (int a = -n; a < n; a++) {
		for (int b = -n; b < n; b++) {
			float choose_mat = random_float_0_1();
			vec3 center(a + 0.9*random_float_0_1(), 0.2, b + 0.9*random_float_0_1());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {  // diffuse
					list.push_back(new Sphere(center, 0.2,
						new Lambertian(vec3(random_float_0_1()*random_float_0_1(),
							random_float_0_1()*random_float_0_1(), random_float_0_1()*random_float_0_1()))));
				}
				else if (choose_mat < 0.95) { // metal
					list.push_back(new Sphere(center, 0.2,
						new Metal(vec3(0.5*(1 + random_float_0_1()), 0.5*(1 + random_float_0_1()), 0.5*(1 + random_float_0_1())),
							0.5*random_float_0_1())));
				}
				else {  // glass
					list.push_back(new Sphere(center, 0.2, new Dielectric(1.5)));
				}
			}
		}
	}

	//list.push_back(new Sphere(vec3(-4, 1, 0), 0.2, new Lambertian(vec3(0.4, 0.2, 0.1))));
	//list.push_back(new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0)));
	//list.push_back(new Sphere(vec3(0, 7, 0), 2.0, new Diffuse_light(vec3(4))));
	//list.push_back(new RectXY(3, 5, 1, 2, -2, new Diffuse_light(vec3(4))));
	list.push_back(new Triangle(vec3(-4, 1, 0), vec3(4, 1, 0), vec3(0, 2, 0), new Lambertian(vec3(0.4, 0.8, 0.1))));
	list.push_back(new Triangle(vec3(-2, 1, 0), vec3(0, 1, 0), vec3(-1, 2, 0), new Lambertian(vec3(0.8, 0.2, 0.1))));
	list.push_back(new Triangle(vec3(0, 1, 0), vec3(1, 1, 0), vec3(0, -1, 0), new Lambertian(vec3(0.4, 0.2, 0.8))));
}

//#define scene_random
#define scene02
void output_ppm()
{
#ifdef scene_random
	/*int nx = 200, ny = 100, ns = 100;
	vec3 left_lower_corner(-2, -1, -1), up(0, 2, 0), right(4, 0, 0), origin(0, 0, 0);
	Camera cam(left_lower_corner, up, right, origin);*/
	int nx = 1200, ny = 800, ns = 10;
	vec3 lookfrom(13, 2, 3), lookat(0, 0, 0), vup(0, 1, 0);
	Camera cam(lookfrom, lookat, vup, 20, float(nx) / float(ny));
	list<Hitable*> list;
	//list.push_back(new Sphere(vec3(-1, 0, -1), -0.45, new Dielectric(1.5)));
	random_scene(list);
	//Hitable_list object_list(list);
	Bvh bvh(list, 0.0, 1.0);
#endif

#ifdef scene01
	int nx = 512, ny = 512, ns = 100;
	vec3 lookfrom(0.0, 0.0, 4),
		lookat(0.0, 0.0, 0.0),
		vup(0.0, 1.0, 0.0);
	Camera cam(lookfrom, lookat, vup, 50., float(nx) / float(ny));
	Object obj("./scenes/Scene01/cup.obj");
	Bvh bvh(obj.scene, 0.0, 1.0);
#endif

#ifdef scene02
	int nx = 512, ny = 512, ns = 10;
	vec3 lookfrom(0.0, 0.0, 4),
		lookat(0.0, 0.0, 0.0),
		vup(0.0, 1.0, 0.0);
	Camera cam(lookfrom, lookat, vup, 50., float(nx) / float(ny));
	Object obj("./scenes/Scene02/room.obj");
	//obj.scene.push_back(new Sphere(vec3(0.0, 1.589, -1.274), 0.2, new Diffuse_light(vec3(50, 50, 40))));
	//Sphere light(vec3(0.0, 1.589, -1.274), 0.2, new Diffuse_light(vec3(50, 50, 40)));
	RectXZ light(-0.2, 0.2, -0.2, 0.2, 1.589, new Diffuse_light(vec3(5, 5, 4)));
	obj.scene.push_back(&light);
	Bvh bvh(obj.scene, 0.0, 1.0);
#endif

	std::ofstream fout("./output2/MC-direct_light.ppm");
	fout << "P3" << endl << nx << " " << ny << endl << 255 << endl; //P is capital
	Performance p;
	p.start();
	for (int j = ny - 1; j >= 0; j--)
	{
		if (j % 10 == 0)
		{
			cout << "line : " << j << "/" << ny << ", compute time " << p.end() << "s." << endl;
			p.start();
		}
		for (int i = 0; i < nx; i++)
		{
			vec3 rgb(0.0);
			for (int s = 0; s < ns; s++)
			{
				float u = (float(i) + random_float_0_1()) / float(nx),
					v = (float(j) + random_float_0_1()) / float(ny);
				rgb += color(cam.get_ray(u, v), bvh, light, 0);
			}
			rgb /= ns;
			rgb = sqrt(rgb);
			fout << int(255.99*rgb[0]) << " " << int(255.99*rgb[1]) << " " << int(255.99*rgb[2]) << endl;
		}
	}
	fout.close();
}

int main(int argc, char *argv[])
{
	Performance p;
	p.start();
	output_ppm();
	cout << "total time: " << p.end() << "s." << endl;
}