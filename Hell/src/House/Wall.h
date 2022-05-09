#pragma once
#include "Header.h"
#include "Physics/PhysX.h"

class Wall
{
public:
	std::vector<Vertex> m_vertices;
	unsigned int m_VAO = 0;
	unsigned int m_VBO = 0;

	Wall();
	void Draw(Shader* shader);
	void BuildMeshFromVertices();
	void CleanUp();

	void AddVerticesClockwise(Vertex v1, Vertex v2, Vertex v3, Vertex v4);
	void AddVerticesCounterClockwise(Vertex v1, Vertex v2, Vertex v3, Vertex v4);

	PxRigidStatic* m_rigidStatic;
	PxTriangleMesh* m_triMesh;
};