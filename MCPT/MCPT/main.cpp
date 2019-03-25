#include <fstream>
//#include <glut.h>
#include <gtc/matrix_transform.hpp>
#include "Material.h"
#include "Camera.h"
#include "obj_loader.h"
#include "Performance.h"
#include "PDF.h"

vec3 color(const Ray& r, Hitable& object_list, Hitable& light, const int depth)
{
	// the color for hit
	hit_record hit_rec;
	if (object_list.hit(r, 0.001, INT_MAX, hit_rec))
	{
		float pdf_value;
		scatter_record scatter_rec;
		vec3 emit = hit_rec.material_ptr->emitted(r, hit_rec);
		//cout << emit << endl;
		if (depth < 50 && hit_rec.material_ptr->scatter(r, hit_rec, scatter_rec))
		{
			//cout << "depth" << depth << endl;
			if (scatter_rec.is_specular)
			{
				/*if (isnan(scatter_rec.specular_ray.direction()[0]))
					cout << "specular_ray direction is nan" << endl;*/
					/*return emit + scatter_rec._albedo *
						color(Ray(hit_rec.p, scatter_rec.pdf_ptr->importance_sampling()),
							object_list, light, depth + 1);*/
				return scatter_rec.albedo * color(scatter_rec.specular_ray, object_list, light, depth + 1);
			}
			else
			{
				std::shared_ptr<PDF_to_light> pdf_light(new PDF_to_light(light, hit_rec.p));
				PDF_mix pdf(pdf_light, scatter_rec.pdf_ptr);
				Ray scattered;
				do
				{
					scattered = Ray(hit_rec.p, pdf.importance_sampling());
					pdf_value = pdf.value(scattered.direction());
					if (isnan(scattered.direction()[0]))
						cout << "s nan" << endl;
					if (isnan(pdf_value))
						cout << "pdf nan" << endl;
				} while (pdf_value == 0);
				/*if (pdf_value == 0)
				{
					vec3 tmp(scattered.direction());
					cout << hit_rec.normal << "¡¤" << tmp << endl;
					cout << dot(hit_rec.normal, scattered.direction()) << " " << pdf_value << endl;
				}*/
				/*if (isnan(emit[0]))
					cout << "emitted is nan" << endl;
				if (isnan(scatter_rec._albedo[0]))
					cout << "albedo is nan" << endl;
				if (isnan(hit_rec.material_ptr->scattering_pdf(r, hit_rec, scattered)))
					cout << "scatter_pdf is nan" << endl;
				if (pdf_value == 0)
					cout << "pdf value" << endl;*/
				return emit + scatter_rec.albedo * hit_rec.material_ptr->scattering_pdf(r, hit_rec, scattered) *
					color(scattered, object_list, light, depth + 1) / pdf_value;
			}
		}
		else
			return emit;
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

	list.push_back(new Sphere(vec3(-4, 1, 0), 0.2, new Lambertian(vec3(0.8, 0.1, 0.1))));
	//list.push_back(new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0)));
	//list.push_back(new Sphere(vec3(0, -1, 0), 2.0, new Diffuse_light(vec3(4))));
	//list.push_back(new RectXY(3, 5, 1, 2, -2, new Diffuse_light(vec3(4))));
	/*list.push_back(new Triangle(vec3(-4, 1, 0), vec3(4, 1, 0), vec3(0, 2, 0), new Lambertian(vec3(0.4, 0.8, 0.1))));
	list.push_back(new Triangle(vec3(-2, 1, 0), vec3(0, 1, 0), vec3(-1, 2, 0), new Lambertian(vec3(0.8, 0.2, 0.1))));
	list.push_back(new Triangle(vec3(0, 1, 0), vec3(1, 1, 0), vec3(0, -1, 0), new Lambertian(vec3(0.4, 0.2, 0.8))));*/
}

bool IsLittleEndian() {
	int a = 0x1234;
	char c = *(char *)&a;
	if (c == 0x34) {
		return true;
	}
	return false;
}

//#define scene_random
#define scene_room
//#define scene_cup
//#define scene_mis
//#define OUTPIUT_PPM
void output_ppm()
{
#ifdef scene_random
	/*int nx = 200, ny = 100, ns = 100;
	vec3 left_lower_corner(-2, -1, -1), up(0, 2, 0), right(4, 0, 0), origin(0, 0, 0);
	Camera cam(left_lower_corner, up, right, origin);*/
	int nx = 1200, ny = 800, ns = 10;
	vec3 lookfrom(13, 2, 3), lookat(0, 0, 0), vup(0, 1, 0);
	Camera cam(lookfrom, lookat, vup, 20, float(nx) / float(ny));
	list<Hitable*> obj_list;
	//list.push_back(new Sphere(vec3(-1, 0, -1), -0.45, new Dielectric(1.5)));
	random_scene(obj_list);
	//Hitable_list object_list(list);
	Sphere light_sphere(vec3(0, -1, 0), 2.0, new Diffuse_light(vec3(4, 4, 3.2)));
	RectXY light_rect(3, 5, 1, 2, -2, new Diffuse_light(vec3(5, 5, 4)));
	obj_list.push_back(&light_sphere);
	obj_list.push_back(&light_rect);
	Bvh bvh(obj_list, 0.0, 1.0);

	list<Hitable*> light_list;
	light_list.push_back(&light_sphere);
	light_list.push_back(&light_rect);
	Hitable_list light(light_list);
	std::string filename = "./output3/random";
#endif

#ifdef scene_room
	int nx = 512, ny = 512, ns = 10;
	vec3 lookfrom(0.0, 0.0, 4),
		lookat(0.0, 0.0, 0.0),
		vup(0.0, 1.0, 0.0);
	Camera cam(lookfrom, lookat, vup, 50., float(nx) / float(ny));
	Object obj("./scenes/Scene01/room.obj");
	Sphere light_sphere(vec3(0.0, 1.589, -1.274), 0.2, new Light(vec3(50, 50, 40)));
	cout << "sceen tri:" << obj.scene.size() << endl << "sample: " << ns << endl;
	obj.scene.push_back(&light_sphere);
	Bvh bvh(obj.scene, 0.0, 1.0);

	// light
	list<Hitable*> list;
	list.push_back(&light_sphere);
	Hitable_list light(list);
	std::string filename = "./room/room";
#endif

#ifdef scene_cup
	int nx = 512, ny = 512, ns = 1000;
	vec3 lookfrom(0.0, 0.64, 0.52),
		lookat(0.0, 0.4, 0.3),
		vup(0.0, 1.0, 0.0);
	Camera cam(lookfrom, lookat, vup, 60., float(nx) / float(ny));
	Object obj("./scenes/Scene02/cup.obj");
	//-2.758771896,1.5246,0
	RectXY light_rect(-2.758771896 - 0.5, -2.758771896 + 0.5, 1.5246 - 0.5, 1.5246 + 0.5, 0, new Light(vec3(40, 40, 40)));
	obj.scene.push_back(&light_rect);
	cout << obj.scene.size() << endl;
	Bvh bvh(obj.scene, 0.0, 1.0);

	// light
	list<Hitable*> light_list;
	light_list.push_back(&light_rect);
	Hitable_list light(light_list);
	std::string filename = "./cup/cup";
#endif

#ifdef scene_mis
	int nx = 1152, ny = 864, ns = 1000;
	vec3 lookfrom(0.0, 2.0, 15),
		lookat(0.0, 1.69521, 14.0476),
		vup(0.0, 0.952421, -0.304787);
	Camera cam(lookfrom, lookat, vup, 28, float(nx) / float(ny));
	Object obj("./scenes/Scene03/VeachMIS.obj");
	Sphere light_sphere1(vec3(-10, 10, 4), 0.5, new Light(vec3(800))),
		light_sphere2(vec3(3.75, 0, 0), 0.033, new Light(vec3(901.803))),
		light_sphere3(vec3(1.25, 0, 0), 0.1, new Light(vec3(100))),
		light_sphere4(vec3(-1.25, 0, 0), 0.3, new Light(vec3(11.1111))),
		light_sphere5(vec3(-3.75, 0, 0), 0.9, new Light(vec3(1.23457)));
	cout << obj.scene.size() << endl;
	obj.scene.push_back(&light_sphere1);
	obj.scene.push_back(&light_sphere2);
	obj.scene.push_back(&light_sphere3);
	obj.scene.push_back(&light_sphere4);
	obj.scene.push_back(&light_sphere5);
	Bvh bvh(obj.scene, 0.0, 1.0);

	// light
	list<Hitable*> list;
	list.push_back(&light_sphere1);
	list.push_back(&light_sphere2);
	list.push_back(&light_sphere3);
	list.push_back(&light_sphere4);
	list.push_back(&light_sphere5);
	Hitable_list light(list);
	std::string filename = "./VeachMIS/VeachMIS";
#endif

	cout << "output: " << filename << endl;
	vector<vec3> rgb(nx*ny, vec3(0.));
	Performance p;
	p.start();
	for (int s = 0; s <= ns; s++)
	{
//#pragma omp parallel for
		for (int j = 0; j < ny; j++)
		{
			for (int i = 0; i < nx; i++)
			{
				int index = nx - 1 - i + (ny - 1 - j) * nx;
				float u = (float(i) + random_float_0_1()) / float(nx),
					v = (float(j) + random_float_0_1()) / float(ny);
				rgb[index] = vec3(s) * rgb[index] + color(cam.get_ray(u, v), bvh, light, 0);
				rgb[index] /= (s + 1);
			}
		}
		if (s % 10 == 0)
		{
			cout << "sample " << s << ", compute time " << p.end() << "s." << endl;
			p.start();
#ifdef OUTPIUT_PPM
			float max_color = 0;
			std::ofstream fout(filename + std::to_string(s) + ".ppm");
			fout << "P3" << endl << nx << " " << ny << endl << 255 << endl; //P is capital
			for (auto c : rgb)
			{
				if (c[0] > max_color)
					max_color = c[0];
				if (c[1] > max_color)
					max_color = c[1];
				if (c[2] > max_color)
					max_color = c[2];
				fout << int(255.99*c[0]) << " " << int(255.99*c[1]) << " " << int(255.99*c[2]) << endl;
			}
			fout.close();
			cout << "max_color : " << int(255.99*max_color) << endl;
#endif
#ifndef OUTPIUT_PPM
			std::ofstream fout(filename + std::to_string(s) + ".pfm", std::ios::out | std::ios::binary);
			fout << "PF" << endl << nx << " " << ny << endl << (IsLittleEndian() ? -1 : 1) << endl;
			for (auto c : rgb)
			{
				fout.write(reinterpret_cast<char *>(&c[0]), sizeof(float));
				fout.write(reinterpret_cast<char *>(&c[1]), sizeof(float));
				fout.write(reinterpret_cast<char *>(&c[2]), sizeof(float));
			}
			fout.close();
#endif
		}
	}
}

int main(int argc, char *argv[])
{
	Performance p;
	p.start();
	output_ppm();
	cout << "total time: " << p.end() << "s." << endl;
}