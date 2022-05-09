#include "GameData.h"

Player GameData::s_player1;
Player GameData::s_player2;
std::vector<Room> GameData::s_rooms;
std::vector<Door> GameData::s_doors;
std::vector<Light> GameData::s_lights;
std::vector<BulletDecal> GameData::s_bulletDecals;
std::vector<BloodDecal> GameData::s_bloodDecals;
std::vector<EntityStatic> GameData::s_staticEntities;
bool GameData::s_splitScreen = true;
std::vector< VolumetricBloodSplatter> GameData::s_volumetricBloodSplatters;
int GameData::s_volumetricBloodObjectsSpawnedThisFrame = 0;
std::vector<BulletCasing> GameData::s_bulletCasings;
std::vector<Controller> GameData::s_controllers;

void GameData::Clear()
{
	s_rooms.clear();
	s_doors.clear();
	s_lights.clear();
	s_bulletDecals.clear();
}

void GameData::Update(float deltaTime)
{
	// Reset some shit
	s_volumetricBloodObjectsSpawnedThisFrame = 0;

	// Update some shit
	for (BulletCasing& casing : s_bulletCasings)			
		casing.Update(deltaTime);

	for (Door& door : s_doors)
		door.Update(deltaTime);

	for (Light& light : s_lights)
		DetermineIfLightNeedsShadowmapUpdate(light);

	for (VolumetricBloodSplatter& vbs : s_volumetricBloodSplatters)
		vbs.Update(deltaTime);

	// Remove volumetric blood older than 0.9 seconds
	for (vector<VolumetricBloodSplatter>::iterator it = s_volumetricBloodSplatters.begin(); it != s_volumetricBloodSplatters.end();) {
		if (it->m_CurrentTime > 0.9)
			it = s_volumetricBloodSplatters.erase(it);		
		else
			++it;
	}
}

void GameData::CreateVolumetricBlood(glm::vec3 position, glm::vec3 rotation, glm::vec3 front)
{
	// Spawn a max of 4 per frame
	if (s_volumetricBloodObjectsSpawnedThisFrame < 4)
		GameData::s_volumetricBloodSplatters.push_back(VolumetricBloodSplatter(position, rotation, front));
	s_volumetricBloodObjectsSpawnedThisFrame++;
}

void GameData::DetermineIfLightNeedsShadowmapUpdate(Light& light)
{
	// Players
	float playerGraceDistance = 0.5f;
	float distanceToP1 = glm::length(GameData::s_player1.GetPosition() - light.m_position) - playerGraceDistance;
	float distanceToP2 = glm::length(GameData::s_player2.GetPosition() - light.m_position) - playerGraceDistance;

	if (distanceToP1 < light.m_radius || distanceToP2 + 1 < light.m_radius) {
		light.m_needsUpadte = true;
		return;
	}

	// Doors
	for (Door& door : GameData::s_doors) {
		float doorGrace = 1;
		float distanceToDoor = glm::length(door.m_transform.position - light.m_position) - doorGrace;

		if (distanceToDoor < light.m_radius && door.m_swing > 0 && door.m_swing < door.m_swingMaxAngle) {
			light.m_needsUpadte = true;
			return;
		}
	}

	// Otherwise, light don't need an update
	light.m_needsUpadte = false;
}

void GameData::DrawInstancedGeometry(Shader* shader)
{
	if (!s_bulletCasings.size())
		return;

	// create GL buffer to store matrices in if ya haven't already
	static unsigned int s_buffer = 0;
	if (s_buffer == 0)
		glGenBuffers(1, &s_buffer);

	// Init shit
	shader->use();
	std::vector<glm::mat4> modelMatrixVector;

	// Glock casings
	for (BulletCasing& casing: s_bulletCasings)
		if (casing.m_type == BulletCasing::CasingType::GLOCK_CASING)
			modelMatrixVector.push_back(casing.m_modelMatrix);


	if (modelMatrixVector.size())
	{
		/**/AssetManager::GetMaterialPtr("BulletCasing")->Bind();
		Model* model = AssetManager::GetModelPtr("BulletCasing");
		glBindBuffer(GL_ARRAY_BUFFER, s_buffer);
		glBufferData(GL_ARRAY_BUFFER, modelMatrixVector.size() * sizeof(glm::mat4), &modelMatrixVector[0], GL_STATIC_DRAW);
		unsigned int VAO = model->m_meshes[0]->VAO;
		unsigned int numIndices = model->m_meshes[0]->indices.size();
		glBindVertexArray(VAO);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(1 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0, modelMatrixVector.size());
	}

	// Shotgun shells
	modelMatrixVector.clear();
	for (BulletCasing& casing : s_bulletCasings)
		if (casing.m_type == BulletCasing::CasingType::SHOTGUN_SHELL)
			modelMatrixVector.push_back(casing.m_modelMatrix); 

	if (modelMatrixVector.size()) 
	{
		AssetManager::GetMaterialPtr("Shell")->Bind();
		Model* model = AssetManager::GetModelPtr("Shell");
		glBindBuffer(GL_ARRAY_BUFFER, s_buffer);
		glBufferData(GL_ARRAY_BUFFER, modelMatrixVector.size() * sizeof(glm::mat4), &modelMatrixVector[0], GL_STATIC_DRAW);
		unsigned int VAO = model->m_meshes[0]->VAO;
		unsigned int numIndices = model->m_meshes[0]->indices.size();
		glBindVertexArray(VAO);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(1 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0, modelMatrixVector.size());
	}
}


struct InstanceData {
	glm::mat4 modelMatrix;
	//glm::vec3 normal;
};

void GameData::DrawInstancedBulletDecals(Shader* shader)
{
	static unsigned int frontFacingPlaneVAO = 0;
	float offset = 0.1f;
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
	
	// create GL buffer to store matrices in if ya haven't already
	static unsigned int instancing_array_buffer = 0;
	if (instancing_array_buffer == 0)
		glGenBuffers(1, &instancing_array_buffer);

	// Init shit
	shader->use();
	std::vector<InstanceData> instanceData;

	for (BulletDecal& decal : s_bulletDecals) {
		InstanceData data;
		data.modelMatrix = decal.m_modelMatrix;
		//data.normal = decal.m_normal;
		instanceData.push_back(data);
	}

	if (instanceData.size())
	{
		glBindBuffer(GL_ARRAY_BUFFER, instancing_array_buffer);
		glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(InstanceData), &instanceData[0], GL_STATIC_DRAW);
		unsigned int VAO = frontFacingPlaneVAO;
		unsigned int numIndices = 6;
		glBindVertexArray(VAO);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE,sizeof(InstanceData), (void*)0);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(1 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(3 * sizeof(glm::vec4)));
		//glEnableVertexAttribArray(9);
		//glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, normal));
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		//glVertexAttribDivisor(9, 1);
		glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0, instanceData.size());
	}
}

void GameData::DrawInstancedBloodDecals(Shader* shader)
{
	/*static unsigned int frontFacingPlaneVAO = 0;
	float offset = 0.1f;
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
	}*/

	// create GL buffer to store matrices in if ya haven't already
	static unsigned int instancing_array_buffer = 0;
	if (instancing_array_buffer == 0)
		glGenBuffers(1, &instancing_array_buffer);

	// Init shit
	shader->use();
	std::vector<glm::mat4> instanceData_type_0;

	for (BloodDecal& blood : s_bloodDecals) {

		if (blood.m_type == 0)
			InstanceData data;
		//data.modelMatrix = blood.m_modelMatrix;
	//	instanceData.push_back(data);
	}

	/*if (instanceData.size())
	{
		glBindBuffer(GL_ARRAY_BUFFER, instancing_array_buffer);
		glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(InstanceData), &instanceData[0], GL_STATIC_DRAW);
		unsigned  int VAO = frontFacingPlaneVAO;
		unsigned int numIndices = 6;
		glBindVertexArray(VAO);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)0);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(1 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(3 * sizeof(glm::vec4)));
		//glEnableVertexAttribArray(9);
		//glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)offsetof(InstanceData, normal));
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);
		//glVertexAttribDivisor(9, 1);
		glBindVertexArray(VAO);
		glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0, instanceData.size());
	}*/
}