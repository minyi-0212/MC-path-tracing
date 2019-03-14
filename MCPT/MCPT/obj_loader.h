#pragma once

#include <vector>
#include <string>
#include <map>
#include <glm.hpp>

struct mtl_param
{
	std::string name;
	float Ns, // shiness
		Ni, // refractive index
		illum;
	glm::vec3 ka, kd, ks, Tf; // ambient diffuse specular transmission filter

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
	void get_faces(std::vector<std::vector<std::vector<int>>>& faces);
	void get_faces(std::vector<std::vector<int>>& faces);
	void get_faces(std::vector<std::vector<glm::vec3>>& faces);

private:
	bool read_mtl(std::string mtl_file);

	std::vector<glm::vec3> _vertices;//顶点
	std::vector<int> _faces;//面
	std::vector<std::pair<float, float>> _texturecoords;//纹理坐标
	std::vector<glm::vec3> _normals;//法向量
	std::vector<std::pair<int, int>> _row_col;//面参数
	std::vector<std::string> _materials;
};