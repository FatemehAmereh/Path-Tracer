#ifndef PLANE_H
#define PLANE_H

#include <stdio.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Material.h"

class Plane {
public:
	glm::vec3 normal;
	glm::vec3 position;
	float lenght;
	Material mtl;

	Plane(glm::vec3 normal, glm::vec3 position, float length, Material mtl) :
		normal(normal),
		position(position),
		lenght(length),
		mtl(mtl) {}
};

#endif