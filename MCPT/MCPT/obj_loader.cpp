#include <fstream>
#include <sstream>
#include <iostream>
#include "obj_loader.h"

using namespace glm;
using namespace std;
bool Object::read_mtl(string mtl_file)
{
	ifstream file(mtl_file);
	if (!file.is_open())
	{
		cerr << "mtl文件" + mtl_file + "无法打开" << endl;
		return false;
	}

	bool flag = false;
	string material_name;
	mtl_param material;

	string type;
	int illumination_model;


	while (file >> type)
	{
		if (type == "newmtl")
		{
			if (flag)
			{
				//material_table.insert(make_pair(material_name, material));
				material = mtl_param();
			}
			else flag = true;

			file >> material_name;
			material.name = material_name;
			cout << material_name << endl;
		}
		else if (type == "Kd")//duffuse 参数
		{
			file >> material.kd.x >> material.kd.y >> material.kd.z;
		}
	}

	//if (flag) material_table.insert(make_pair(material_name, material));//插入最后一种材料

	return true;
}

void Object::read_obj(string obj_file)
{
	string line, type, mtllib, mtlname;
	ifstream fin(obj_file);
	istringstream is;

	if (!fin) {
		cout << "Cannot open the obj file :" << obj_file << endl;
		exit(0);
	}
	while (!fin.eof())
	{
		is.clear();
		getline(fin, line);
		if (line.size() == 0 || line[0] == '#' || line[0] == 's')
			continue;
		is.str(line.c_str());
		is >> type;
		//存储mtl文件名称
		if (type == "mtllib") {
			is >> mtllib;
			string mtl_path = obj_file.substr(0, obj_file.find_last_of('/') + 1) + mtllib;

			if (!read_mtl(mtl_path))
			{
				cerr << "Failed to read mtl file" << mtl_path << endl;
				break;
			}
		}
		//存储材质名称
		else if (type == "usemtl") {
			is >> mtlname;
			_materials.push_back(mtlname);
		}
		//存储点坐标
		else if (type == "v") {
			vec3 v;
			is >> v.x >> v.y >> v.z;
			_vertices.push_back(v);
		}
		//存储法向量
		else if (type == "vn") {
			vec3 vn;
			is >> vn.x >> vn.y >> vn.z;
			_normals.push_back(vn);
		}
		//存储纹理坐标
		else if (type == "vt") {
			pair<float, float>vt;
			is >> vt.first >> vt.second;
			_texturecoords.push_back(vt);
		}
		//新的一块
		else if (type == "g" || type == "o") {
			pair<int, int> rc;
			rc.second = 0;
			_row_col.push_back(rc);
			continue;
		}
		//存储面信息
		else if (type == "f") {
			int r = 0, c = 0;
			while (is >> type) {
				c = count(type.begin(), type.end(), '/');
				switch (c) {
				case 0:
					_faces.push_back(atoi(type.c_str()));
					break;
				case 1:
					_faces.push_back(atoi(string(type.begin(), type.begin() + type.find("/")).c_str()));
					_faces.push_back(atoi(string(type.begin() + type.find("/") + 1, type.end()).c_str()));
					break;
				case 2:
					int a1 = type.find("/");
					int a2 = type.find("/", a1 + 1);
					_faces.push_back(atoi(string(type.begin(), type.begin() + a1).c_str()));
					_faces.push_back(atoi(string(type.begin() + a1 + 1, type.begin() + a2).c_str()));
					_faces.push_back(atoi(string(type.begin() + a2 + 1, type.end()).c_str()));
					break;
				}
				++r;
			}
			//f 286/540/182 283/535/182 285/538/182 287/541/182 
			//col=3,row=4
			pair<int, int> rc;
			rc.first = r;
			rc.second = c + 1;
			_row_col.push_back(rc);
		}
		line = "";
	}
	fin.close();
}

void Object::get_faces(vector<vector<int>>& faces)
{
	/*faces.resize(_row_col.size());
	int index = 0;
	for (int i = 0; i < _row_col.size(); i++)
	{
		faces[i].resize(_row_col[i].first, 0);
		for (int x = 0; x < _row_col[i].first; x++)
		{
			faces[i][x] = _faces[index];
			index += _row_col[i].second;
		}
	}*/
	int index = 0, face_size = 0;
	for (int i = 0; i < _row_col.size(); i++)
	{
		if (_row_col[i].first == 0)
			continue;
		std::vector<int> tmp(_row_col[i].first, 0);
		for (int x = 0; x < _row_col[i].first; x++)
		{
			tmp[x] = _faces[index];
			index += _row_col[i].second;
		}
		faces.push_back(tmp);
	}
};

void Object::get_faces(vector<vector<vec3>>& faces)
{
	int index = 0, face_size = 0;
	for (int i = 0; i < _row_col.size(); i++)
	{
		if (_row_col[i].first == 0)
			continue;
		std::vector<vec3> tmp(_row_col[i].first);
		for (int x = 0; x < _row_col[i].first; x++)
		{
			tmp[x] = _vertices[_faces[index] - 1];
			index += _row_col[i].second;
		}
		faces.push_back(tmp);
	}
};

void Object::get_faces(vector<vector<vector<int>>>& faces)
{
	faces.clear();
	int poly_size = 0, index = 0;
	for (int id = 0; id < _row_col.size(); id++)
	{
		if (_row_col[id].first == 0)
		{
			faces.resize(++poly_size);
		}
		else
		{
			vector<int> tmp(_row_col[id].first);
			for (int x = 0; x < _row_col[id].first; x++)
			{
				tmp[x] = _faces[index];
				index += _row_col[id].second;
			}
			faces[poly_size - 1].push_back(tmp);
		}
	}

	/*faces.resize(_row_col.size());
	int index = 0;
	for (int i = 0; i < _row_col.size(); i++)
	{
		faces[i].resize(_row_col[i].first, 0);
		for (int x = 0; x < _row_col[i].first; x++)
		{
			faces[i][x] = _faces[index];
			index += _row_col[i].second;
		}
	}*/
};
