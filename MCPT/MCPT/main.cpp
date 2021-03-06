#include <fstream>
#include <gtc/matrix_transform.hpp>
#include <direct.h>
#include <time.h>
#include "Material.h"
#include "Camera.h"
#include "obj_loader.h"
#include "Performance.h"
#include "PDF.h"

vec3 background = vec3(0);
vec3 color(const Ray& r, Hitable& object_list, Hitable& light, const int depth)
{
	// the color for hit
	hit_record hit_rec;
	if (object_list.hit(r, RAY_MIN_T, INT_MAX, hit_rec))
	{
		float pdf_value;
		scatter_record scatter_rec;
		vec3 emit = hit_rec.material_ptr->emitted(r, hit_rec);
		//cout << emit << endl;
		if (depth < 50 && hit_rec.material_ptr->scatter(r, hit_rec, scatter_rec))
		{
			if (scatter_rec.status == 2)
			{
				return scatter_rec.albedo * color(scatter_rec.scatter_ray, object_list, light, depth + 1);
			}
			else if (scatter_rec.status == 1)
			{
				Ray reflected(hit_rec.p, normalize(reflect(r.direction(), scatter_rec.pdf_ptr->importance_sampling())));
				return scatter_rec.albedo * hit_rec.material_ptr->scattering_pdf_value_for_blinn_phone(r, hit_rec, reflected) *
					color(reflected, object_list, light, depth + 1);
			}
			else if (scatter_rec.status == 0)
			{
				std::shared_ptr<PDF_to_light> pdf_light(new PDF_to_light(light, hit_rec.p));
				PDF_mix pdf(pdf_light, scatter_rec.pdf_ptr);
				Ray scattered;
				do
				{
					scattered = Ray(hit_rec.p, pdf.importance_sampling());
					pdf_value = pdf.value(scattered.direction());
				} while (pdf_value == 0);
				return emit + scatter_rec.albedo * hit_rec.material_ptr->scattering_pdf(r, hit_rec, scattered) *
					color(scattered, object_list, light, depth + 1) / pdf_value;
			}
		}
		else
			return emit;
	}
	else
		return background;
	/*vec3 unit_direction = r.direction();
	float t = unit_direction[0] == 0 && unit_direction[1] == 0 && unit_direction[2] == 0 ?
	0.5 : 0.5*(normalize(unit_direction)[1] + 1.0);
	return vec3(1.0 - t, 1.0 - t, 1.0 - t) + vec3(t*0.5, t*0.7, t*1.0);*/
}

void random_scene(list<Hitable*>& list)
{
	int n = 5;
	list.push_back(new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(vec3(0.5))));
	for (int a = -n; a < n; a++) {
		for (int b = -n; b < n; b++) {
			float choose_mat = random_float_0_1();
			vec3 center(a + 0.9*random_float_0_1(), 0.2, b + 0.9*random_float_0_1());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				if (choose_mat < 0.8) {  // diffuse
					list.push_back(new Sphere(center, center[1],
						new Lambertian(vec3(random_float_0_1()*random_float_0_1(),
							random_float_0_1()*random_float_0_1(), random_float_0_1()*random_float_0_1()))));
				}
				else if (choose_mat < 0.95) { // metal
					list.push_back(new Sphere(center, center[1],
						new Metal(vec3(0.5*(1 + random_float_0_1()), 0.5*(1 + random_float_0_1()), 0.5*(1 + random_float_0_1())),
							0.5*random_float_0_1())));
				}
				else {  // glass
					list.push_back(new Sphere(center, center[1], new Dielectric(1.5)));
				}
			}
		}
	}

	list.push_back(new Sphere(vec3(-4, 0.7, 2), 0.7, new Lambertian(vec3(0.6, 0.3, 0.1))));
	list.push_back(new Sphere(vec3(6, 0.6, 1), 0.6, new Dielectric(1.5)));
	list.push_back(new Sphere(vec3(0, 0.8, 0), 0.8, new Metal(vec3(0.7, 0.6, 0.5), 0.0)));
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

//#define OUTPIUT_PPM
void render_scene_random(const char* path, int ns = 1000, int output_ns = 100)
{
	int nx = 512, ny = 512;
	background = vec3(0.5, 0.7, 1.0);
	vec3 lookfrom(13, 2, 3), lookat(0, 0, 0), vup(0, 1, 0);
	Camera cam(lookfrom, lookat, vup, 20, float(nx) / float(ny));
	list<Hitable*> obj_list;
	random_scene(obj_list);
	Sphere light_sphere(vec3(10, 10, 0), 2.0, new Light(vec3(40, 40, 40)));
	RectXZ light_rect(3, 5, 1, 2, -2, new Light(vec3(50, 50, 40)));
	obj_list.push_back(&light_sphere);
	obj_list.push_back(&light_rect);
	Bvh bvh(obj_list, 0.0, 1.0);

	list<Hitable*> light_list;
	light_list.push_back(&light_sphere);
	light_list.push_back(&light_rect);
	Hitable_list light(light_list);
	char output_path[MAX_PATH], output_file[MAX_PATH];
	sprintf_s(output_path, "%s/random", path);
	cout << "output: " << output_path << endl;
	_mkdir(output_path);

	vector<vec3> rgb(nx*ny, vec3(0.));
	Performance p;
	p.start();
	for (int s = 0; s <= ns; s++)
	{
#pragma omp parallel for
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
		if (s % output_ns == 0)
		{
			cout << "sample " << s << ", compute time " << p.end() << "s." << endl;
			p.start();
#ifdef OUTPIUT_PPM
			float max_color = 0;
			sprintf_s(output_file, "%s/random%d.ppm", output_path, s);
			std::ofstream fout(output_file);
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
			sprintf_s(output_file, "%s/random%d.pfm", output_path, s);
			std::ofstream fout(output_file, std::ios::out | std::ios::binary);
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

void render_scene_room(const char* path, int ns = 1000, int output_ns = 100)
{
	int nx = 512, ny = 512;
	vec3 lookfrom(0.0, 0.0, 4),
		lookat(0.0, 0.0, 0.0),
		vup(0.0, 1.0, 0.0);
	Camera cam(lookfrom, lookat, vup, 50., float(nx) / float(ny));
	Object obj("./scenes/Scene01/room.obj");
	Sphere light_sphere(vec3(0.0, 1.589, -1.274), 0.2, new Light(vec3(50, 50, 40)));
	cout << "tri num: " << obj.scene.size() << endl
		<< "sample: " << output_ns << "," << ns << endl;
	obj.scene.push_back(&light_sphere);
	Bvh bvh(obj.scene, 0.0, 1.0);

	// light
	list<Hitable*> list;
	list.push_back(&light_sphere);
	Hitable_list light(list);
	char output_path[MAX_PATH], output_file[MAX_PATH];
	sprintf_s(output_path, "%s/room", path);
	cout << "output: " << output_path << endl;
	_mkdir(output_path);

	vector<vec3> rgb(nx*ny, vec3(0.));
	Performance p;
	p.start();
	for (int s = 0; s <= ns; s++)
	{
#pragma omp parallel for
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
		if (s % output_ns == 0)
		{
			cout << "sample " << s << ", compute time " << p.end() << "s." << endl;
			p.start();
#ifdef OUTPIUT_PPM
			float max_color = 0;
			sprintf_s(output_file, "%s/room%d.ppm", output_path, s);
			std::ofstream fout(output_file);
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
			sprintf_s(output_file, "%s/room%d.pfm", output_path, s);
			std::ofstream fout(output_file, std::ios::out | std::ios::binary);
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

void render_scene_cup(const char* path, int ns = 1000, int output_ns = 100)
{
	int nx = 512, ny = 512;
	vec3 lookfrom(0.0, 0.64, 0.52),
		lookat(0.0, 0.4, 0.3),
		vup(0.0, 1.0, 0.0);
	Camera cam(lookfrom, lookat, vup, 60., float(nx) / float(ny));
	Object obj("./scenes/Scene02/cup.obj");
	RectYZ light_rect(1.5246 - 0.5, 1.5246 + 0.5, 0 - 0.5, 0 + 0.5, -2.7587, new Light(vec3(40, 40, 40)));
	cout << "tri num: " << obj.scene.size() << endl
		<< "sample: " << output_ns << "," << ns << endl;
	obj.scene.push_back(&light_rect);
	Bvh bvh(obj.scene, 0.0, 1.0);

	// light
	list<Hitable*> light_list;
	light_list.push_back(&light_rect);
	Hitable_list light(light_list);
	char output_path[MAX_PATH], output_file[MAX_PATH];
	sprintf_s(output_path, "%s/cup", path);
	cout << "output: " << output_path << endl;
	_mkdir(output_path);

	vector<vec3> rgb(nx*ny, vec3(0.));
	Performance p;
	p.start();
	for (int s = 0; s <= ns; s++)
	{
#pragma omp parallel for
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
		if (s % output_ns == 0)
		{
			cout << "sample " << s << ", compute time " << p.end() << "s." << endl;
			p.start();
#ifdef OUTPIUT_PPM
			float max_color = 0;
			sprintf_s(output_file, "%s/cup%d.ppm", output_path, s);
			std::ofstream fout(output_file);
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
			sprintf_s(output_file, "%s/cup%d.pfm", output_path, s);
			std::ofstream fout(output_file, std::ios::out | std::ios::binary);
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

void render_scene_mis(const char* path, int ns = 1000, int output_ns = 100)
{
	int nx = 1152, ny = 864;
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
	cout << "tri num: " << obj.scene.size() << endl
		<< "sample: " << output_ns << "," << ns << endl;
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
	char output_path[MAX_PATH], output_file[MAX_PATH];
	sprintf_s(output_path, "%s/VeachMIS", path);
	cout << "output: " << output_path << endl;
	_mkdir(output_path);

	vector<vec3> rgb(nx*ny, vec3(0.));
	Performance p;
	p.start();
	for (int s = 0; s <= ns; s++)
	{
#pragma omp parallel for
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
		if (s % output_ns == 0)
		{
			cout << "sample " << s << ", compute time " << p.end() << "s." << endl;
			p.start();
#ifdef OUTPIUT_PPM
			float max_color = 0;
			sprintf_s(output_file, "%s/VeachMIS%d.ppm", output_path, s);
			std::ofstream fout(output_file);
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
			sprintf_s(output_file, "%s/VeachMIS%d.pfm", output_path, s);
			std::ofstream fout(output_file, std::ios::out | std::ios::binary);
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

void render_scene_my(const char* path, int ns = 1000, int output_ns = 100)
{
	int nx = 512, ny = 512;
	vec3 lookfrom(0.0, 0.0, 4),
		lookat(0.0, 0.0, 0.0),
		vup(0.0, 1.0, 0.0);
	Camera cam(lookfrom, lookat, vup, 50., float(nx) / float(ny));
	Object obj("./scenes/Scene04/test.obj");
	obj.scene.push_back(new Sphere(vec3(1, -1, -1), 0.2, new Dielectric(1.5)));
	obj.scene.push_back(new RectXY(-1.8, 1.8, -1.8, 1.8, -1.9, new Metal(vec3(1.0), 0.0)));
	Sphere light_sphere(vec3(0.0, 1.589, -1.274), 0.2, new Light(vec3(50, 50, 40)));
	cout << "tri num: " << obj.scene.size() << endl
		<< "sample: " << output_ns << "," << ns << endl;
	obj.scene.push_back(&light_sphere);
	Bvh bvh(obj.scene, 0.0, 1.0);

	// light
	list<Hitable*> list;
	list.push_back(&light_sphere);
	Hitable_list light(list);
	char output_path[MAX_PATH], output_file[MAX_PATH];
	sprintf_s(output_path, "%s/my", path);
	cout << "output: " << output_path << endl;
	_mkdir(output_path);

	vector<vec3> rgb(nx*ny, vec3(0.));
	Performance p;
	p.start();
	for (int s = 0; s <= ns; s++)
	{
#pragma omp parallel for
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
		if (s % output_ns == 0)
		{
			cout << "sample " << s << ", compute time " << p.end() << "s." << endl;
			p.start();
#ifdef OUTPIUT_PPM
			float max_color = 0;
			sprintf_s(output_file, "%s/my%d.ppm", output_path, s);
			std::ofstream fout(output_file);
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
			sprintf_s(output_file, "%s/my%d.pfm", output_path, s);
			std::ofstream fout(output_file, std::ios::out | std::ios::binary);
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
	std::string path(".");
	srand((unsigned)time(NULL));
	Performance p;
	p.start();
	if (argc == 1) {
		cout << "need the argument for the scene!" << endl;
		return 0;
	}
	if (argc == 3)
	{
		path = argv[2];
	}

	if (!strcmp(argv[1], "room"))
		render_scene_room(path.c_str(), 5000, 100);
	else if (!strcmp(argv[1], "cup"))
		render_scene_cup(path.c_str(), 5000, 100);
	else if (!strcmp(argv[1], "VeachMIS"))
		render_scene_mis(path.c_str(), 10000, 100);
	else if (!strcmp(argv[1], "my"))
		render_scene_my(path.c_str(), 5000, 100);
	else
		render_scene_random(path.c_str(), 500, 100);
	cout << "total time: " << p.end() << "s." << endl;
	//system("pause");
}