#include "Wall.h"
#include "Helpers/Util.h"

Wall::Wall()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
}

void Wall::DeleteBuffers()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
}

void Wall::AddVerticesClockwise(Vertex v1, Vertex v2, Vertex v3, Vertex v4)
{
	Util::SetNormalsAndTangentsFromVertices(&v1, &v2, &v3);
	Util::SetNormalsAndTangentsFromVertices(&v3, &v4, &v1);

	v1.Bitangent = glm::vec3(0, 1, 0);
	v2.Bitangent = glm::vec3(0, 1, 0);
	v3.Bitangent = glm::vec3(0, 1, 0);
	v4.Bitangent = glm::vec3(0, 1, 0);
	m_vertices.push_back(v1);
	m_vertices.push_back(v2);
	m_vertices.push_back(v3);
	m_vertices.push_back(v3);
	m_vertices.push_back(v4);
	m_vertices.push_back(v1);
}

void Wall::AddVerticesCounterClockwise(Vertex v1, Vertex v2, Vertex v3, Vertex v4)
{
	Util::SetNormalsAndTangentsFromVertices(&v3, &v2, &v1);
	Util::SetNormalsAndTangentsFromVertices(&v1, &v4, &v3);

	v1.Bitangent = glm::vec3(0, 1, 1);
	v2.Bitangent = glm::vec3(0, 1, 1);
	v3.Bitangent = glm::vec3(0, 1, 1);
	v4.Bitangent = glm::vec3(0, 1, 1);
	m_vertices.push_back(v3);
	m_vertices.push_back(v2);
	m_vertices.push_back(v1);
	m_vertices.push_back(v1);
	m_vertices.push_back(v4);
	m_vertices.push_back(v3);
}

void Wall::BuildMeshFromVertices()
{
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

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

	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	
}

void Wall::Draw(Shader* shader)
{
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
}
