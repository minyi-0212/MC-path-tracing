#pragma once

#include <vector>
#include <string>
#include <map>
#include <glm.hpp>
#include "Hitable.h"
#include "Material.h"

struct mtl_param
{
	std::string name;
	float Ns, // shiness
		Ni; // refractive index
	int illum;
	glm::vec3 Ka, Kd, Ks, Tf; // ambient diffuse specular transmission filter

};

class Object
{
public:
	Object(std::string filename) { read_obj(filename); };
	void read_obj(std::string obj_file);
	void get_vertices(std::vector<glm::vec3>& vertices)
	{
		vertices = _vertices;
	};
	void get_scene(list<Hitable*>& scene)
	{
		std::copy(scene.begin(), scene.end(), std::back_inserter(scene));
	};
	list<Hitable*> scene;

private:
	bool read_mtl(std::string mtl_file);

	std::vector<glm::vec3> _vertices;//顶点
	std::vector<glm::vec3> _normals;//法向量
	std::map<std::string, mtl_param*> _mtl_map;
};