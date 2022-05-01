#pragma once
#include "Header.h"
#include "Core/Player.h"
#include "Helpers/AssetManager.h"
#include "House/Room.h"
#include "House/Door.h"
#include "House/Light.h"
#include "Core/EntityStatic.h"
#include "rapidjson/document.h"
#include <rapidjson/filereadstream.h>
#include "Effects/BulletDecal.h"
#include "Effects/BloodDecal.h"
#include "Effects/VolumetricBloodSplatter.h"
#include "Effects/BulletCasing.h"
#include "Core/Controller.h"


class GameData {
public:
	static Player s_player1;
	static Player s_player2;
	static std::vector<Room> s_rooms;
	static std::vector<Door> s_doors;
	static std::vector<Light> s_lights;	
	static std::vector<EntityStatic> s_staticEntities;

	static std::vector<BulletDecal> s_bulletDecals;
	static std::vector<BloodDecal> s_bloodDecals;
	static std::vector<VolumetricBloodSplatter> s_volumetricBloodSplatters;
	static std::vector<BulletCasing> s_bulletCasings;

	static void Clear();
	static void Update(float deltaTime);

	static void CreateVolumetricBlood(glm::vec3 position, glm::vec3 rotation, glm::vec3 front);
	static void DetermineIfLightNeedsShadowmapUpdate(Light& light);

	static void DrawInstanced(Shader* shader);

	static bool s_splitScreen;

	static std::vector<Controller> s_controllers;
		
	// internals
private:
	static int s_volumetricBloodObjectsSpawnedThisFrame;
};