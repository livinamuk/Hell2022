#pragma once
#include "Header.h"

class BulletDecal
{
public:
	BulletDecal();

	glm::vec3 m_position;
	glm::vec3 m_normal;
	float m_randomRotation;

	void Draw(Shader* shader);
};

