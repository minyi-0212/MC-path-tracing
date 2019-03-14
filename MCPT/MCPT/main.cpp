#include <iostream>
#include <fstream>
//#include <glut.h>
#include <gtc/matrix_transform.hpp>
#include <math.h>
#include "Material.h"
#include "Camera.h"
#include "obj_loader.h"
#include "Performance.h"

using std::cout;
using std::endl;

vec3 color(const Ray& r, Hitable& object_list, const int depth)
{
	// the color for hit
	hit_record rec;
	if (object_list.hit(r, 0.001, INT_MAX, rec))
	{
		Ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.material_ptr->scatter(r, rec, attenuation, scattered))
		{
			//cout <<depth << " : " << attenuation[0]<< attenuation[1]<< attenuation[2] << endl;
			return rec.material_ptr->emitted() + attenuation * color(scattered, object_list, depth + 1);
		}
		else
			return rec.material_ptr->emitted();
	}
	else
		return vec3(0);
}

void random_scene(list<Hitable*>& list)
{
	int n = 0;
	list.push_back(new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(vec3(0.5))));
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

	//list.push_back(new Sphere(vec3(0, 1, 0), 1.0, new Dielectric(1.5)));
	list.push_back(new Sphere(vec3(0, 7, 0), 2.0, new Diffuse_light(vec3(4))));
	list.push_back(new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(vec3(0.4, 0.2, 0.1))));
	list.push_back(new Sphere(vec3(4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0)));
	list.push_back(new RectXY(3, 5, 1, 2, -2, new Diffuse_light(vec3(4))));
}

void output_ppm()
{
	/*int nx = 200, ny = 100, ns = 100;
	vec3 left_lower_corner(-2, -1, -1), up(0, 2, 0), right(4, 0, 0), origin(0, 0, 0);
	Camera cam(left_lower_corner, up, right, origin);*/
	int nx = 1200, ny = 800, ns = 100;
	vec3 lookfrom(13,2,3), lookat(0, 0, 0), vup(0, 1, 0);
	Camera cam(lookfrom, lookat, vup, 20, float(nx)/float(ny));
	list<Hitable*> list;
	list.push_back(new Sphere(vec3(-1, 0, -1), -0.45, new Dielectric(1.5)));
	random_scene(list);
	//Hitable_list object_list(list);
	Bvh bvh(list, 0.0, 1.0);
	
	std::ofstream fout("./output2/light.ppm");
	fout << "P3" << endl << nx << " " << ny << endl << 255 << endl; //P is capital
	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			vec3 rgb(0.0);
			for (int s = 0; s < ns; s++)
			{
				float u = (float(i) + random_float_0_1()) / float(nx),
					v = (float(j) + random_float_0_1()) / float(ny);
				rgb += color(cam.get_ray(u, v), bvh, 0);
			}
			rgb /= ns;
			rgb = sqrt(rgb);
			fout << int(255.99*rgb[0]) << " " << int(255.99*rgb[1]) << " " << int(255.99*rgb[2]) << endl;
		}
	}
	fout.close();
}

#include <set>
int main(int argc, char *argv[])
{
	Performance p;
	p.start();
	output_ppm();
	
	// test obj_loader
	//{
	//	Object obj("./scenes/Scene01/cup.obj");
	//	/*vector<vector<vec3>> faces;
	//	vector<vec3> vertex;
	//	obj.get_faces(faces);
	//	obj.get_vertices(vertex);
	//	cout <<"vertex :"<< vertex.size() << endl;
	//	std::set<int> f;
	//	for (auto face : faces)
	//		f.insert(face.size());
	//	cout << "face :" << faces.size() << "-";
	//	for (auto ff : f)
	//		cout << ff << " ";
	//	cout << endl;*/
	//}
	cout << "total time: " << p.end() << "s." << endl;
}