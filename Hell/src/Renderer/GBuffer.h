#pragma once
#include "Header.h"

class GBuffer
{
public:
	GBuffer(int width, int height);
	GBuffer();
	~GBuffer();

	// gAlbedo: alpha channel has gun mask, used for using the indirect light shadowmap for gun models instead of reg shadowmap.
	// gEmissve

	unsigned int ID;
	unsigned int gAlbedo, gNormal, gRMA, gFinal, rboDepth, gLighting, gPostProcessed, gEmissive;

	void Configure(int width, int height);
};
