#include "room.h"
#include "Helpers/AssetManager.h"
#include "earcut/earcut.hpp"
#include <array>
#include "core/GameData.h"

// looks like new rooms and old rooms may be sharing VAO
	// also you DO need to recreate the polygon incase the new vertex location results in different shape or whatever

Room::Room()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
}

void Room::DrawFloor(Shader* shader)
{
	AssetManager::GetMaterialPtr("FloorBoards")->Bind();

	shader->setMat4("model", glm::mat4(1));
	shader->setInt("u_TEXCOORD_FLAG", 1);

	glCullFace(GL_FRONT);
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glCullFace(GL_BACK);

	shader->setInt("u_TEXCOORD_FLAG", 0);
}

void Room::DrawCeiling(Shader* shader)
{
	AssetManager::GetMaterialPtr("PlasterCeiling")->Bind();

	shader->setMat4("model", Transform(glm::vec3(0, 2.4f, 0)).to_mat4());
	shader->setInt("u_TEXCOORD_FLAG", 1);

	//glCullFace(GL_FRONT);
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	//glCullFace(GL_BACK);

	shader->setInt("u_TEXCOORD_FLAG", 0);
}

void Room::DrawWalls(Shader* shader)
{
	AssetManager::GetMaterialPtr("WallPaper")->Bind();

	shader->setMat4("model", glm::mat4(1));
	for (Wall& wall : m_walls)
		wall.Draw(shader);
}


float MapRange(float OldValue, float OldMin, float OldMax, float NewMin, float NewMax)
{
	return (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin;
}

bool PointIsOnLineSegment(glm::vec3 p, glm::vec3 q, glm::vec3 r)
{
	if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
		q.z <= max(p.z, r.z) && q.z >= min(p.z, r.z))
		return true;

	return false;
}

void Room::BuildMeshFromVertices()
{
	// Generate floor polygon vertices 
	using Point = std::array<double, 2>;
	std::vector<std::vector<Point>> polygon;

	std::vector<Point> a;
	polygon.push_back(a);

	for (glm::vec3& p : m_vertices) {
		Point point;
		point[0] = p.x;
		point[1] = p.z;
		polygon[0].push_back(point);
	}

	m_indices = mapbox::earcut<unsigned int>(polygon);


	// Now go for it
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindVertexArray(m_VAO);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(glm::vec3), &m_vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glBindVertexArray(0);

	ComputeWindingOrder();

	// Remove any old walls
	for (Wall& wall : m_walls)
		wall.DeleteBuffers();
	m_walls.clear();
	
	// Add the beginning vertex to the end to prevent an empty hole. You will remove it later.
	m_vertices.push_back(m_vertices[0]);

	for (int i = 0; i < m_vertices.size() - 1; i++)
	{
		// Create the wall
		m_walls.push_back(Wall());
		Wall* wall = &m_walls[m_walls.size() - 1];

		// Init shit
		bool finishedBuildingWall = false;
		glm::vec3 cursor = m_vertices[i];
		glm::vec3 wallStart = m_vertices[i];
		glm::vec3 wallEnd = m_vertices[i+1];

		// v2     v3 //
		//           //
		// v1     v4 //


		// For each vertex, walk towards the next (from i to i+1)
		// and check do a line intersection test against the SIDES of EVERY door

		while (!finishedBuildingWall)
		{
			for (Door& door : GameData::s_doors)
			{
				if (PointIsOnLineSegment(door.GetVert1(), wallStart, wallEnd)) {
					std::cout << "vert 1 hit found\n";
				}
				if (PointIsOnLineSegment(door.GetVert2(), wallStart, wallEnd)) {
					std::cout << "vert 2 hit found\n";
				}
				if (PointIsOnLineSegment(door.GetVert3(), wallStart, wallEnd)) {
					std::cout << "vert 3 hit found\n";
				}
				if (PointIsOnLineSegment(door.GetVert4(), wallStart, wallEnd)) {
					std::cout << "vert 4 hit found\n";
				}
			}
			finishedBuildingWall = true;
		}


		

		if (false)
		{
			Vertex v1, v2, v3, v4;

			v1.Position = m_vertices[i];
			v2.Position = m_vertices[i] + glm::vec3(0, 2.4f, 0);
			v3.Position = m_vertices[i + (int)1] + glm::vec3(0, 2.4f, 0);;
			v4.Position = m_vertices[i + (int)1];

			float wallWidth = glm::length((v4.Position - v1.Position)) / 2.4f;

			v1.TexCoords = glm::vec2(0, 0);
			v2.TexCoords = glm::vec2(0, 1);
			v3.TexCoords = glm::vec2(wallWidth, 1);
			v4.TexCoords = glm::vec2(wallWidth, 0);

			if (m_sumOfEdges > 0) {
				wall->m_vertices.push_back(v1);
				wall->m_vertices.push_back(v2);
				wall->m_vertices.push_back(v3);
				wall->m_vertices.push_back(v3);
				wall->m_vertices.push_back(v4);
				wall->m_vertices.push_back(v1);
			}
			else
			{
				wall->m_vertices.push_back(v3);
				wall->m_vertices.push_back(v2);
				wall->m_vertices.push_back(v1);
				wall->m_vertices.push_back(v1);
				wall->m_vertices.push_back(v4);
				wall->m_vertices.push_back(v3);
			}
		}
		
		wall->BuildMeshFromVertices();
	}
	// remove that temporary final element of the vertices vector that u added
	m_vertices.pop_back();
}

void Room::ComputeWindingOrder()
{
	float total = 0;

	for (int i = 0; i < m_vertices.size() - 1; i++) {
		float x1 = m_vertices[i].x;
		float z1 = m_vertices[i].z;
		float x2 = m_vertices[i + 1].x;
		float z2 = m_vertices[i + 1].z;
		total += (x2 - x1) * (z2 + z1);
	}
	float x1 = m_vertices[m_vertices.size() - 1].x;
	float z1 = m_vertices[m_vertices.size() - 1].z;
	float x2 = m_vertices[0].x;
	float z2 = m_vertices[0].z;
	total += (x2 - x1) * (z2 + z1);

	m_sumOfEdges = total;

	if (m_invertWallNormals)
		m_sumOfEdges *= -1;
}

void Room::DeleteAllData()
{
	// Remove any old walls
	for (Wall& wall : m_walls)
		wall.DeleteBuffers();
	m_walls.clear();

	// Remove GL buffer objects
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);
}

