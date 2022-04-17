#pragma once
#include "Header.h"

#define ENV_MAP_WIDTH 128
#define ENV_MAP_HEIGHT 128

class EnvMap
{
	public: // methods
		EnvMap();
		void Init();

	public: // fields
		glm::vec3 m_position;
		unsigned int m_FboID = 0;
		unsigned int m_DepthTexID = 0;
		unsigned int m_TexID = 0;
		unsigned int SH_FboID = 0;
		unsigned int SH_TexID = 0;
		unsigned int depthCubemap;
		//bool m_needsNewCubeMap = true; 
};
