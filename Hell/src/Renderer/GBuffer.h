#pragma once
#include "Header.h"

class GBuffer
{
public:
	GBuffer(int width, int height);
	GBuffer();
	~GBuffer();

	unsigned int ID;
	unsigned int gAlbedo, gNormal, gRMA, gFinal, rboDepth, gLighting, gPostProcessed;

	void Configure(int width, int height);
};
