#include "EnvMap.h"

EnvMap::EnvMap()
{
	//Init();
}

void EnvMap::Init()
{
	glGenFramebuffers(1, &m_FboID);
	glGenTextures(1, &m_DepthTexID);
	glGenTextures(1, &m_TexID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthTexID);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, ENV_MAP_WIDTH, ENV_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_2D); // Allocate the mipmaps
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexID, 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TexID);

	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, ENV_MAP_WIDTH, ENV_MAP_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D); // Allocate the mipmaps
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_TexID, 0);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TexID);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	auto glstatus = glGetError();
	if (glstatus != GL_NO_ERROR)
		std::cout << "Error in GL call: " << glstatus << std::endl;


	// Sphereical Harmonics shit (FBO with single color 3x3 RGB_16F attachment)
	glGenFramebuffers(1, &SH_FboID);
	glGenTextures(1, &SH_TexID);
	glBindFramebuffer(GL_FRAMEBUFFER, SH_FboID);
	glBindTexture(GL_TEXTURE_2D, SH_TexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 3, 3, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SH_TexID, 0);
}
