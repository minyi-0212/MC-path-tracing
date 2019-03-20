#include "common.h"

std::ostream& operator << (std::ostream& output, vec3& c) //定义运算符“<<”重载函数
{
	output << "(" << c[0] << "," << c[1] << "," << c[2] << ")";
	return output;
}