#include "GBuffer.h"

GBuffer::GBuffer(int width, int height)
{
	glGenFramebuffers(1, &ID);
	glGenTextures(1, &gAlbedo);
	glGenTextures(1, &gNormal);
	glGenTextures(1, &gRMA);
	glGenTextures(1, &gFinal);
	glGenTextures(1, &gLighting);
	glGenTextures(1, &gPostProcessed);
	glGenTextures(1, &rboDepth); 
	Configure(width, height); 
}

GBuffer::GBuffer()
{
}

GBuffer::~GBuffer()
{
}

void GBuffer::Configure(int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, ID);

	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedo, 0);

	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	glBindTexture(GL_TEXTURE_2D, gRMA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gRMA, 0);

	glBindTexture(GL_TEXTURE_2D, gFinal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gFinal, 0);

	glBindTexture(GL_TEXTURE_2D, gLighting);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gLighting, 0);
	
	glBindTexture(GL_TEXTURE_2D, gPostProcessed);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gPostProcessed, 0);
	
	//unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
//glDrawBuffers(5, attachments);

	glBindTexture(GL_TEXTURE_2D, rboDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, rboDepth, 0);

	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (fboStatus == 36053)
		std::cout << "G Buffer: " << "COMPLETE\n";
	if (fboStatus == 36054)
		std::cout << "G Buffer: " << "INCOMPLETE ATTACHMENT\n";
	if (fboStatus == 36057)
		std::cout << "G Buffer: " << "INCOMPLETE DIMENSIONS\n";
	if (fboStatus == 36055)
		std::cout << "G Buffer: " << "MISSING ATTACHMENT\n";
	if (fboStatus == 36061)
		std::cout << "G Buffer: " << "UNSPORTED\n";

	auto glstatus = glGetError();
	if (glstatus != GL_NO_ERROR)
		std::cout << "Error in GL call: " << glstatus << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}