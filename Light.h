#ifndef LIGHT_H
#define LIGHT_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Light {
public:
	glm::vec3 position;
	glm::vec3 intensity;

	Light(glm::vec3 pos, glm::vec3 i) {
		position = pos;
		intensity = i;
	}
};

#endif