#include <fstream>
#include <sstream>
#include "obj_loader.h"

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
	MTL* material = nullptr;
	//Material* mtl;
	string type;
	int illumination_model;

	cout << "mtl: " << endl;
	while (file >> type)
	{
		if (type == "newmtl")
		{
			file >> material_name;
			material = new MTL();
			material->para.name = material_name;
			_mtl_map.insert(make_pair(material_name, material));
			cout << "---" << material_name << endl;
		}
		else if (type == "Ka")// ambient
		{
			file >> material->para.Ka.x >> material->para.Ka.y >> material->para.Ka.z;
		}
		else if (type == "Kd")// duffuse
		{
			file >> material->para.Kd.x >> material->para.Kd.y >> material->para.Kd.z;
		}
		else if (type == "Ks")// specular
		{
			file >> material->para.Ks.x >> material->para.Ks.y >> material->para.Ks.z;
		}
		else if (type == "Tf")// transmission
		{
			file >> material->para.Tf.x >> material->para.Tf.y >> material->para.Tf.z;
		}
		else if (type == "Ni")// transmission
		{
			file >> material->para.Ni;
		}
		else if (type == "Ns")// transmission
		{
			file >> material->para.Ns;
		}
		else if (type == "illum")// transmission
		{
			file >> material->para.illum;
		}
		else
		{
			cout << type << " not parsing." << endl;
		}
	}
	cout << endl;

	/*material = new MTL();
	material_name = "lambert11SG";
	material->para.name = "lambert11SG";
	material->para.Kd = vec3(0.67, 0.67, 0.67);
	material->para.Ka = vec3(0.00, 0.00, 0.00);
	material->para.Tf = vec3(1.00, 1.00, 1.00);
	material->para._Ni = 1.00;
	_mtl_map.insert(make_pair(material_name, material));*/

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
	MTL* material = nullptr;

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
			material = _mtl_map.find(mtlname)->second;
			cout << "    " << mtlname << " " << material->para.Kd << endl;
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
				/*if (!(mtlname == "lambert2SG"))
					break;*/
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
					/*scene.push_back(new Triangle(_vertices[vertex_idx[0] - 1], _vertices[vertex_idx[1] - 1],
						_vertices[vertex_idx[2] - 1], new Lambertian(vec3(1,0,0))));*/
					scene.push_back(new Triangle(_vertices[vertex_idx[0] - 1], _vertices[vertex_idx[1] - 1],
						_vertices[vertex_idx[2] - 1], material));

					vertex_idx[1] = vertex_idx[2];
					vertex_normal_idx[1] = vertex_normal_idx[2];
					index = 2;
				}
			}
		}
	}
	fin.close();
}