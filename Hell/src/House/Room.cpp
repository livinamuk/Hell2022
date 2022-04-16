#include "room.h"
#include "Helpers/AssetManager.h"
#include "earcut/earcut.hpp"
#include <array>
#include "core/GameData.h"

// looks like new rooms and old rooms may be sharing VAO
	// also you DO need to recreate the polygon incase the new vertex location results in different shape or whatever

Room::Room()
{
	glGenVertexArrays(1, &m_floor_VAO);
	glGenBuffers(1, &m_floor_VBO);
	glGenBuffers(1, &m_floor_EBO);
	glGenVertexArrays(1, &m_ceiling_VAO);
	glGenBuffers(1, &m_ceiling_VBO);
	glGenBuffers(1, &m_ceiling_EBO);
}

void Room::DrawFloor(Shader* shader)
{
	AssetManager::GetMaterialPtr("FloorBoards")->Bind();

	glCullFace(GL_FRONT);
	shader->setMat4("model", glm::mat4(1));
	shader->setInt("u_TEXCOORD_FLAG", 1);
	glBindVertexArray(m_floor_VAO);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	shader->setInt("u_TEXCOORD_FLAG", 0);
	glCullFace(GL_BACK);
}

void Room::DrawCeiling(Shader* shader)
{
	AssetManager::GetMaterialPtr("PlasterCeiling")->Bind();

	shader->setMat4("model", Transform(glm::vec3(0, 2.4f, 0)).to_mat4());
	shader->setInt("u_TEXCOORD_FLAG", 1);
	glBindVertexArray(m_ceiling_VAO);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	shader->setInt("u_TEXCOORD_FLAG", 0);
}

void Room::DrawWalls(Shader* shader)
{
	// Actual walls
	AssetManager::GetMaterialPtr("WallPaper")->Bind();
	shader->setMat4("model", glm::mat4(1));
	for (Wall& wall : m_walls)
		wall.Draw(shader);

	// Floor trims
	for (glm::mat4& m : m_floorTrimMatrices) {
		AssetManager::GetMaterialPtr("Trims")->Bind();
		AssetManager::m_models["TrimFloor"].Draw(shader, m);
	}
	// Celing trims
	for (glm::mat4& m : m_ceilingTrimMatrices) {
		AssetManager::GetMaterialPtr("Trims")->Bind();
		AssetManager::m_models["TrimCeiling"].Draw(shader, m);
	}
}


float MapRange(float OldValue, float OldMin, float OldMax, float NewMin, float NewMax)
{
	return (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin;
}

bool PointIsOnLineSegment(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
	if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
		q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
		return true;

	return false;
}

/*
int orientation(glm::vec2 p, glm::vec2 q, glm::vec2 r)
{
	// See https://www.geeksforgeeks.org/orientation-3-ordered-points/
	// for details of below formula.
	int val = (q.y - p.y) * (r.x - q.x) -
		(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0;  // collinear

	return (val > 0) ? 1 : 2; // clock or counterclock wise
}

bool doIntersect(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2)
{
	// Find the four orientations needed for general and
	// special cases
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	// General case
	if (o1 != o2 && o3 != o4)
		return true;

	// Special Cases
	// p1, q1 and p2 are collinear and p2 lies on segment p1q1
	if (o1 == 0 && PointIsOnLineSegment(p1, p2, q1)) return true;

	// p1, q1 and q2 are collinear and q2 lies on segment p1q1
	if (o2 == 0 && PointIsOnLineSegment(p1, q2, q1)) return true;

	// p2, q2 and p1 are collinear and p1 lies on segment p2q2
	if (o3 == 0 && PointIsOnLineSegment(p2, p1, q2)) return true;

	// p2, q2 and q1 are collinear and q1 lies on segment p2q2
	if (o4 == 0 && PointIsOnLineSegment(p2, q1, q2)) return true;

	return false; // Doesn't fall in any of the above cases
}

struct IntersectResult {
	glm::vec2 point;
	bool intersection = false;
};

IntersectResult LineIntersection(glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D)
{
	IntersectResult result;

	// Line AB represented as a1x + b1y = c1
	double a1 = B.y - A.y;
	double b1 = A.x - B.x;
	double c1 = a1 * (A.x) + b1 * (A.y);

	// Line CD represented as a2x + b2y = c2
	double a2 = D.y - C.y;
	double b2 = C.x - D.x;
	double c2 = a2 * (C.x) + b2 * (C.y);

	double determinant = a1 * b2 - a2 * b1;

	if (determinant == 0)
	{
		// The lines are parallel. This is simplified
		// by returning a pair of FLT_MAX
		return result;
	}
	else
	{
		double x = (b2 * c1 - b1 * c2) / determinant;
		double y = (a1 * c2 - a2 * c1) / determinant;
		result.intersection = true;
		result.point = glm::vec2(x, y);
		return result;
	}
}

*/
void Room::BuildMeshFromVertices()
{
	// Generate floor polygon vertices 
	using Point = std::array<double, 2>;
	std::vector<std::vector<Point>> polygon;

	std::vector<Point> a;
	polygon.push_back(a);

	std::vector<Vertex> floorVertices;
	std::vector<Vertex> ceilingVertices;

	for (glm::vec3& p : m_vertices) {

		// ceiling
		Vertex vert;
		vert.Position = p;
		vert.Normal = glm::vec3(0, -1, 0);
		vert.Tangent = glm::vec3(0, -1, 0);
		vert.Bitangent = glm::vec3(1, 0, 0);
		ceilingVertices.push_back(vert);
		// floor
		vert.Position = p;
		vert.Normal = glm::vec3(0, 1, 0);
		vert.Tangent = glm::vec3(1, 0, 0);
		vert.Bitangent = glm::vec3(0, 1, 0);
		floorVertices.push_back(vert);

		Point point;
		point[0] = p.x;
		point[1] = p.z;
		polygon[0].push_back(point);
	}

	m_indices = mapbox::earcut<unsigned int>(polygon);


	// Now go for it
	glBindVertexArray(m_ceiling_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_ceiling_VBO);
	glBufferData(GL_ARRAY_BUFFER, ceilingVertices.size() * sizeof(Vertex), &ceilingVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ceiling_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);		
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));		
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(5, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, MaterialID));
	glEnableVertexAttribArray(5);
	glBindVertexArray(0);

	glBindVertexArray(m_floor_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_floor_VBO);
	glBufferData(GL_ARRAY_BUFFER, floorVertices.size() * sizeof(Vertex), &floorVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_floor_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	glEnableVertexAttribArray(4);
	glVertexAttribIPointer(5, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, MaterialID));
	glEnableVertexAttribArray(5);
	glBindVertexArray(0);

	ComputeWindingOrder();

	// Remove any old walls
	for (Wall& wall : m_walls)
		wall.DeleteBuffers();
	m_walls.clear();

	m_floorTrimMatrices.clear();
	m_ceilingTrimMatrices.clear();
	
	// Add the beginning vertex to the end to prevent an empty hole. You will remove it later.
	//m_vertices.push_back(m_vertices[0]);

	for (int i = 0; i < m_vertices.size(); i++)
	{
		int j = (i + 1) % m_vertices.size();
		//std::cout << "\n";
		//std::cout << "vertex: " << i << "\n";

		// Create the wall
		m_walls.push_back(Wall());
		Wall* wall = &m_walls[m_walls.size() - 1];

		// Init shit
		bool finishedBuildingWall = false;
		glm::vec3 wallStart = m_vertices[i];
		glm::vec3 wallEnd = m_vertices[j];
		glm::vec3 cursor = wallStart;
		glm::vec3 wallDir = glm::normalize(wallEnd - cursor);

		// v2     v3 //
		//           //
		// v1     v4 //


		// For each vertex, walk towards the next (from i to i+1)
		// and check do a line intersection test against the SIDES of EVERY door
		int count = 0;
		while (!finishedBuildingWall || count > 1000)
		//for (int i = 0; i < 15; i++)
		{
			count++;
			float shortestDistance = 9999;
			Door* closestDoor = nullptr;
			glm::vec3 intersectionPoint;

			for (Door& door : GameData::s_doors)
			{;
				// Left side
				glm::vec3 v1(door.GetVert1(glm::vec3(0, 0, -0.15f)));
				glm::vec3 v2(door.GetVert2(glm::vec3(0, 0, 0.15f)));

				// Right side
				glm::vec3 v3(door.GetVert3(glm::vec3(0, 0, 0.15f)));
				glm::vec3 v4(door.GetVert4(glm::vec3(0, 0, -0.15f)));

				// If an intersection is found closer than one u have already then store it
				glm::vec3 tempIntersectionPoint;
				if (Util::LineIntersects(v1, v2, cursor, wallEnd, &tempIntersectionPoint))
				{
					if (shortestDistance > glm::distance(cursor, tempIntersectionPoint)) {

						shortestDistance = glm::distance(cursor, tempIntersectionPoint);
						closestDoor = &door;
						intersectionPoint = tempIntersectionPoint;
					}
				}
				// Check the other side now
				if (Util::LineIntersects(v3, v4, cursor, wallEnd, &tempIntersectionPoint))
				{
					if (shortestDistance > glm::distance(cursor, tempIntersectionPoint)) {

						shortestDistance = glm::distance(cursor, tempIntersectionPoint);
						closestDoor = &door;
						intersectionPoint = tempIntersectionPoint;
					}
				}				
			}

			// Did ya find a door?
			if (closestDoor != nullptr)
			{				
				// The wall piece from cursor to door
				 {
					Vertex v1, v2, v3, v4;
					v1.Position = cursor;
					v2.Position = cursor + glm::vec3(0, 2.4f, 0);
					v3.Position = intersectionPoint + glm::vec3(0, 2.4f, 0);;
					v4.Position = intersectionPoint;
					v1.Normal = glm::vec3(0, 0, 1);
					v2.Normal = glm::vec3(0, 0, 1);
					v3.Normal = glm::vec3(0, 0, 1);
					v4.Normal = glm::vec3(0, 0, 1);

					float wallWidth = glm::length((v4.Position - v1.Position)) / 2.4f;
					float wallHeight = glm::length((v2.Position - v1.Position)) / 2.4f;
					v1.TexCoords = glm::vec2(0, 0);
					v2.TexCoords = glm::vec2(0, wallHeight);
					v3.TexCoords = glm::vec2(wallWidth, wallHeight);
					v4.TexCoords = glm::vec2(wallWidth, 0);



					if (m_sumOfEdges > 0)
						wall->AddVerticesClockwise(v1, v2, v3, v4);
					else
						wall->AddVerticesCounterClockwise(v1, v2, v3, v4);

					// trims
					Transform floorTrans;
					floorTrans.position = (intersectionPoint);
					floorTrans.rotation.y = Util::YRotationBetweenTwoPoints(intersectionPoint, cursor);
					floorTrans.scale.x = glm::distance(cursor, intersectionPoint);
					m_floorTrimMatrices.push_back(floorTrans.to_mat4());
				}

				// The piece above door
				{
					Vertex v1, v2, v3, v4;
					v1.Position = intersectionPoint + glm::vec3(0, 2.0f, 0);;
					v2.Position = intersectionPoint + glm::vec3(0, 2.4f, 0);
					v3.Position = intersectionPoint + (wallDir * (DOOR_WIDTH + 0.05f)) + glm::vec3(0, 2.4f, 0);
					v4.Position = intersectionPoint + (wallDir * (DOOR_WIDTH + 0.05f)) + glm::vec3(0, 2.0f, 0);
					
					float wallWidth = glm::length((v4.Position - v1.Position)) / 2.4f;
					float wallHeight = glm::length((v2.Position - v1.Position)) / 2.4f;
					v1.TexCoords = glm::vec2(0, 0);
					v2.TexCoords = glm::vec2(0, wallHeight);
					v3.TexCoords = glm::vec2(wallWidth, wallHeight);
					v4.TexCoords = glm::vec2(wallWidth, 0);

					if (m_sumOfEdges > 0)
						wall->AddVerticesClockwise(v1, v2, v3, v4);
					else
						wall->AddVerticesCounterClockwise(v1, v2, v3, v4);
				}

				// This 0.05 is so you don't get an intersection with the door itself
				cursor = intersectionPoint + (wallDir * (DOOR_WIDTH + 0.05f));
			}
			// You're on the final bit of wall then aren't ya
			else
			{
					Vertex v1, v2, v3, v4;
					v1.Position = cursor;
					v2.Position = cursor + glm::vec3(0, 2.4f, 0);
					v3.Position = wallEnd + glm::vec3(0, 2.4f, 0);;
					v4.Position = wallEnd;

					float wallWidth = glm::length((v4.Position - v1.Position)) / 2.4f;
					float wallHeight = glm::length((v2.Position - v1.Position)) / 2.4f;
					v1.TexCoords = glm::vec2(0, 0);
					v2.TexCoords = glm::vec2(0, wallHeight);
					v3.TexCoords = glm::vec2(wallWidth, wallHeight);
					v4.TexCoords = glm::vec2(wallWidth, 0);

					if (m_sumOfEdges > 0)
						wall->AddVerticesClockwise(v1, v2, v3, v4);
					else
						wall->AddVerticesCounterClockwise(v1, v2, v3, v4);

					finishedBuildingWall = true;

					// trims
					Transform floorTrans;
					floorTrans.position = (wallEnd);
					floorTrans.rotation.y = Util::YRotationBetweenTwoPoints(wallEnd, cursor);
					floorTrans.scale.x = glm::distance(cursor, wallEnd);
					m_floorTrimMatrices.push_back(floorTrans.to_mat4());

					Transform ceilingTrans;
					ceilingTrans.position = (wallEnd);
					ceilingTrans.rotation.y = Util::YRotationBetweenTwoPoints(wallEnd, wallStart);
					ceilingTrans.scale.x = glm::distance(wallStart, wallEnd);
					m_ceilingTrimMatrices.push_back(ceilingTrans.to_mat4());
			}

			//if (closestDoor == nullptr)
			//	finishedBuildingWall = true;
		}	
		wall->BuildMeshFromVertices();
	}
	// remove that temporary final element of the vertices vector that u added
	//m_vertices.pop_back();
	//m_vertices.pop_back();
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
	glDeleteVertexArrays(1, &m_floor_VAO);
	glDeleteBuffers(1, &m_floor_VBO);
	glDeleteBuffers(1, &m_floor_EBO);
	glDeleteVertexArrays(1, &m_ceiling_VAO);
	glDeleteBuffers(1, &m_ceiling_VBO);
	glDeleteBuffers(1, &m_ceiling_EBO);
}

