#include "GBuffer.h"

GBuffer::GBuffer(int width, int height)
{
	glGenFramebuffers(1, &ID);
	glGenTextures(1, &gAlbedo);
	glGenTextures(1, &gFinal);
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

	glBindTexture(GL_TEXTURE_2D, gFinal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gFinal, 0);

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, attachments);

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