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

void GameData::Clear()
{
	s_rooms.clear();
	s_doors.clear();
	s_lights.clear();
	s_bulletDecals.clear();
}

void GameData::Update(float deltaTime)
{
	for (Door& door : s_doors)
		door.Update(deltaTime);

	for (Light& light : s_lights)
		DetermineIfLightNeedsShadowmapUpdate(light);
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

