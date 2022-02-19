#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Renderer/Mesh.h"
#include "Header.h"
#include "Physics/PhysX.h"

class Model
{
public:
	Model();
	Model(const char* filepath, bool createCollisionMesh = false);
	~Model();

	void ReadFromDisk(bool createCollisionMesh);
	void LoadMeshDataToGL();
	void Draw(Shader* shader, glm::mat4 modelMatrix, int primitiveType = GL_TRIANGLES);
	void DrawMesh(Shader* shader, int meshIndex, glm::mat4 modelMatrix, int primitiveType = GL_TRIANGLES);
	
public:
	std::vector<Mesh*> m_meshes;

public:
	std::string name;
	std::string m_filePath;
	//bool m_hasTexCoords = false;
	bool m_readFromDisk = false;
	bool m_loadedToGL = false;
	FileType m_fileType;

	bool m_hasCollisionMesh = false;
	PxTriangleMesh* m_triMesh;
};
