#pragma once
#include "Header.h"
#include "earcut/earcut.hpp"
#include <array>
#include "Wall.h"

class Room 
{
public:
	std::vector<glm::vec3> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Wall> m_walls;

	float m_sumOfEdges = 0;
	unsigned int m_VAO = 0;
	unsigned int m_VBO = 0;
	unsigned int m_EBO = 0;
	bool m_invertWallNormals = false;

	Room();
	void DrawFloor(Shader* shader);
	void DrawCeiling(Shader* shader);
	void DrawWalls(Shader* shader);
	void BuildMeshFromVertices();
	void ComputeWindingOrder();
	void DeleteAllData();
};
