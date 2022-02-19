#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>

class Transform
{
public: // fields
	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(1);

public: // methods
	glm::mat4 to_mat4(); 
	Transform();
	Transform(glm::vec3 position);
	Transform(glm::vec3 position, glm::vec3 rotation);
	Transform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
	Transform(glm::mat4 matrix);

	static Transform from_mat4(glm::mat4 matrix);
};