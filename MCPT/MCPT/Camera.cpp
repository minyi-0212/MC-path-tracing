#include "Camera.h"
Camera::Camera(vec3& lookfrom, vec3& lookat, vec3& vup, float vfov, float aspect_ratio)
	:origin(lookfrom)
{
	float half_height = tan(vfov * PI / 360),
		half_width = aspect_ratio * half_height;
	vec3 w(normalize(lookfrom - lookat)),
		u(normalize(cross(vup, w))),
		v(cross(w, u));
	left_lower_corner = lookfrom - half_width * u - half_height * v - w;
	up = 2 * half_height*v;
	right = 2 * half_width*u;
}

#include <iostream>
Ray Camera::get_ray(float u, float v)
{
	vec3 tmp(left_lower_corner + u * right + v * up - origin);
	return Ray(origin, left_lower_corner + u * right + v * up - origin);
}