#pragma once
#include "Header.h"

class BloodDecal
{
public:
	Transform m_transform;
	//glm::vec3 m_position;
	glm::vec3 m_normal;
	float m_randomRotation;
	int m_type;
	void Draw(Shader* shader);
};

