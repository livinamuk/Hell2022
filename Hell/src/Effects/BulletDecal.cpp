#include "BulletDecal.h"
#include "Helpers/AssetManager.h"
#include <math.h>

BulletDecal::BulletDecal(glm::vec3 position, glm::vec3 normal)
{	
	Transform translation;
	translation.position = position;
	translation.scale = glm::vec3(0.02f);

	Transform rotationTransform;
	rotationTransform.rotation.z = Util::RandomFloat(0, HELL_PI * 2);

	m_normal = normal;
	glm::vec3 n = m_normal;

	float sign = copysignf(1.0f, n.z);
	const float a = -1.0f / (sign + n.z);
	const float b = n.x * n.y * a;
	glm::vec3 b1 = glm::vec3(1.0f + sign * n.x * n.x * a, sign * b, -sign * n.x);
	glm::vec3 b2 = glm::vec3(b, sign + n.y * n.y * a, -n.y);

	glm::mat4 rotationMatrix = glm::mat4(1);
	rotationMatrix[0] = glm::vec4(b1, 0);
	rotationMatrix[1] = glm::vec4(b2, 0);
	rotationMatrix[2] = glm::vec4(n, 0);

	m_modelMatrix = translation.to_mat4() * rotationMatrix * rotationTransform.to_mat4();
}

void BulletDecal::Draw(Shader* shader)
{
	glDisable(GL_CULL_FACE);

	static unsigned int frontFacingPlaneVAO = 0;


	glDepthFunc(GL_LEQUAL);

	float offset = 0.1f;

	// Setup if you haven't already
	if (frontFacingPlaneVAO == 0) {
		Vertex vert0, vert1, vert2, vert3;
		vert0.Position = glm::vec3(-0.5, 0.5, offset);
		vert1.Position = glm::vec3(0.5, 0.5f, offset);
		vert2.Position = glm::vec3(0.5, -0.5, offset);
		vert3.Position = glm::vec3(-0.5, -0.5, offset);
		vert0.TexCoords = glm::vec2(0, 1);
		vert1.TexCoords = glm::vec2(1, 1);
		vert2.TexCoords = glm::vec2(1, 0);
		vert3.TexCoords = glm::vec2(0, 0);
		vert0.Normal = glm::vec3(0, 0, 1);
		vert1.Normal = glm::vec3(0, 0, 1);
		vert2.Normal = glm::vec3(0, 0, 1);
		vert3.Normal = glm::vec3(0, 0, 1);
		vert0.Bitangent = glm::vec3(0, 1, 0);
		vert1.Bitangent = glm::vec3(0, 1, 0);
		vert2.Bitangent = glm::vec3(0, 1, 0);
		vert3.Bitangent = glm::vec3(0, 1, 0);
		vert0.Tangent = glm::vec3(1, 0, 0);
		vert1.Tangent = glm::vec3(1, 0, 0);
		vert2.Tangent = glm::vec3(1, 0, 0);
		vert3.Tangent = glm::vec3(1, 0, 0);
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		unsigned int i = (unsigned int)vertices.size();
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);
		indices.push_back(i + 2);
		indices.push_back(i + 3);
		indices.push_back(i);
		vertices.push_back(vert0);
		vertices.push_back(vert1);
		vertices.push_back(vert2);
		vertices.push_back(vert3);
		unsigned int VBO;
		unsigned int EBO;
		glGenVertexArrays(1, &frontFacingPlaneVAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindVertexArray(frontFacingPlaneVAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, MaterialID));
	}
	// Draw
	glBindVertexArray(frontFacingPlaneVAO);
	shader->setMat4("model", m_modelMatrix);
	shader->setVec3("u_decalNormal", m_normal);
	shader->setVec3("u_decalPos", m_modelMatrix[3]);
	glDrawElements(GL_TRIANGLES, 8, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
