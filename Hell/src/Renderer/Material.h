#pragma once
#include "Header.h"

class Material
{
public: // methods
	void Bind();
	void BindToSecondSlot();

public: // fields
	std::string name;
	GLuint ALB = 0;
	GLuint NRM = 0;
	GLuint RMA = 0;
	GLuint RME = 0;
};
