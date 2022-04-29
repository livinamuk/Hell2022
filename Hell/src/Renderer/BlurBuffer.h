#pragma once
#include "Header.h"

class BlurBuffer
{
public:
	BlurBuffer(int width, int height);
	BlurBuffer();
	~BlurBuffer();

	//unsigned int ID[2];
	//unsigned int colorBuffers[2];

	unsigned int ID;
	unsigned int textureA;
	unsigned int textureB;
	float width, height;

	void Configure(int width, int height);
};
