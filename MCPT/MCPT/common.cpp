#include "common.h"

std::ostream& operator << (std::ostream& output, vec3& c) //�����������<<�����غ���
{
	output << "(" << c[0] << "," << c[1] << "," << c[2] << ")";
	return output;
}