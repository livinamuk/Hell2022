#pragma once
#include "Header.h"

class BulletDecal
{
public:
	BulletDecal(glm::vec3 position, glm::vec3 normal);
	glm::vec3 m_normal;
	glm::mat4 m_modelMatrix;

	void Draw(Shader* shader);
};

