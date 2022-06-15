#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Material {
public:
	bool diffuse;
	bool metallic;
	glm::vec3 attenuation;

	Material(bool diffuse, bool metallic, glm::vec3 attenuation) :
		diffuse(diffuse),
		metallic(metallic),
		attenuation(attenuation){}
};

#endif