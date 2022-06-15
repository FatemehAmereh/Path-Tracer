#ifndef SPHERE_H
#define SPHERE_H

#include <stdio.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Material.h"

class Sphere {
public:
	glm::vec3 center;
	float radius;
	Material mtl;

	Sphere(glm::vec3 center, float radius, Material mtl):
		center(center),
		radius(radius),
		mtl(mtl) {}
};

#endif