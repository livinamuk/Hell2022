#pragma once
#include "Header.h"

class BloodDecal
{
public:
	// methods
	BloodDecal(Transform transform, int type);
	void Draw(Shader* shader);

public: 
	// fields
	Transform m_transform;
	Transform m_localOffset;
	int m_type;
	glm::mat4 m_modelMatrix;
};

