#include <fstream>
#include <sstream>
#include <iostream>
#include "obj_loader.h"
#include "Material.h"

using namespace glm;
using namespace std;
bool Object::read_mtl(string mtl_file)
{
	ifstream file(mtl_file);
	if (!file.is_open())
	{
		cerr << "Cannot open the mtl file: " + mtl_file << endl;
		return false;
	}

	bool flag = false;
	string material_name;
	mtl_param material;
	string type;
	int illumination_model;

	cout << "mtl: " << endl;
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
			cout << "---" << material_name << endl;
		}
		else if (type == "Ka")// ambient
		{
			file >> material.Ka.x >> material.Ka.y >> material.Ka.z;
		}
		else if (type == "Kd")// duffuse
		{
			file >> material.Kd.x >> material.Kd.y >> material.Kd.z;
		}
		else if (type == "Ks")// specular
		{
			file >> material.Ks.x >> material.Ks.y >> material.Ks.z;
		}
		else if (type == "Tf")// transmission
		{
			file >> material.Tf.x >> material.Tf.y >> material.Tf.z;
		}
		else if (type == "Ni")// transmission
		{
			file >> material.Ni;
		}
		else if (type == "Ns")// transmission
		{
			file >> material.Ns;
		}
		else if (type == "illum")// transmission
		{
			file >> material.illum;
		}
		else
		{
			cout << type << " not parsing." << endl;
		}
	}
	cout << endl;

	//if (flag) material_table.insert(make_pair(material_name, material));//插入最后一种材料

	return true;
}

void Object::read_obj(string obj_file)
{
	string line, type, mtllib, mtlname;
	vec3 v, vn;
	int vertex_idx[3];
	int vertex_normal_idx[3];

	ifstream fin(obj_file);
	istringstream is;
	Material* material;

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
		// load mtl file
		if (type == "mtllib") 
		{
			is >> mtllib;
			string mtl_path = obj_file.substr(0, obj_file.find_last_of('/') + 1) + mtllib;

			if (!read_mtl(mtl_path))
			{
				cerr << "Failed to read mtl file" << mtl_path << endl;
				break;
			}
		}
		// new block
		else if (type == "g" || type == "o")
		{
			is >> type;
			if (type != "default")
				cout << "obj: " << type << endl;
			/*if (type == "table") 
				break;*/
		}
		// the using mtl name
		else if (type == "usemtl")
		{
			is >> mtlname;
			cout << "    " << mtlname << endl;
		}
		// vertex
		else if (type == "v")
		{
			is >> v.x >> v.y >> v.z;
			_vertices.push_back(v);
		}
		// vertex normal
		else if (type == "vn") 
		{
			is >> vn.x >> vn.y >> vn.z;
			_normals.push_back(vn);
		}
		// face
		else if (type == "f") 
		{
			char *p, *next = nullptr;
			char t[100];
			const char *d = " /";
			int index = 0;
			//f 7/13/13 5/14/14 3/3/15 1/1/16
			while (is >> type) // 7/13/13
			{
				//cout << type << endl;
				strcpy_s(t, type.c_str());
				p = strtok_s(t, d, &next);
				if (p)
				{
					vertex_idx[index] = atoi(p);
					p = strtok_s(NULL, d, &next);
					if (p)
					{
						p = strtok_s(NULL, d, &next);
						if (p)
						{
							vertex_normal_idx[index] = atoi(p);
						}
					}
				}
				index++;
				if (index >= 3)
				{
					scene.push_back(new Triangle(_vertices[vertex_idx[0] - 1], _vertices[vertex_idx[1] - 1],
						_vertices[vertex_idx[2] - 1], new Lambertian(vec3(random_float_0_1(), random_float_0_1(), random_float_0_1()))));

					vertex_idx[1] = vertex_idx[2];
					vertex_normal_idx[1] = vertex_normal_idx[2];
					index = 2;
				}
			}
		}
	}
	fin.close();
}