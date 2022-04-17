#pragma once
#include "Header.h"

#define SHADOW_MAP_SIZE 1024
#define SHADOW_NEAR_PLANE 0.1f
#define SHADOW_FAR_PLANE 20.0f	// change this to be the lights radius

class ShadowMap
{
public: // Methods
	void Init();

public: // Fields
	unsigned int m_ID;
	unsigned int m_depthTexture;
	std::vector<glm::mat4> m_projectionTransforms;
};
