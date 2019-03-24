#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm.hpp>
#include <iostream>

using glm::vec3;
using std::cout;
using std::endl;

#define random_float_0_1() rand()/double(RAND_MAX)

std::ostream& operator << (std::ostream& output, vec3& c);