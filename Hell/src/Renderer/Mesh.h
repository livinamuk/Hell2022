#pragma once

#include "Header.h"
#include "Renderer/Transform.h"
#include "Renderer/Shader.h"

class Mesh 
{
public: // methods
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, const char* name);
	void Draw(int primitiveType = GL_TRIANGLES);
	void SetupMesh();

public: // fields
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	unsigned int VAO = 0;
	const char* name;

	int materialID_Set0 = -1;
	int materialID_Set1 = -1;

private: // fields
	unsigned int VBO, EBO;
};
