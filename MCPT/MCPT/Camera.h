#pragma once
#include "Ray.h"
#define PI 3.141592654
class Camera
{
public:
	Camera(vec3& left_lower_corner, vec3& up, vec3& right, vec3& origin) :
		left_lower_corner(left_lower_corner), up(up), right(right), origin(origin) {}
	Camera(vec3& lookfrom, vec3& lookat, vec3& vup,
		float vfov, float aspect_ratio);
	Ray get_ray(float u, float v);

private:
	vec3 left_lower_corner, up, right, origin;
};