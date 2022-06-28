#pragma once
#include "Header.h"
#include "Core/Player.h"
#include "Helpers/AssetManager.h"
#include "House/Room.h"
#include "House/Door.h"
#include "House/Light.h"
#include "House/Window.h"
#include "Core/EntityStatic.h"
#include "rapidjson/document.h"
#include <rapidjson/filereadstream.h>
#include "Effects/BulletDecal.h"
#include "Effects/BloodDecal.h"
#include "Effects/VolumetricBloodSplatter.h"
#include "Effects/BulletCasing.h"
#include "Core/Controller.h"

struct Bullet {
	glm::vec3 origin;
	glm::vec3 direction;
	float force;
	float damage;
	int parentPlayerIndex;
	glm::vec3 parentCameraRotation;
};

class GameData {
public:
	static Player s_player1;
	static Player s_player2;
	static Player s_player3;
	static Player s_player4;
	static int s_playerCount;
	static std::vector<Room> s_rooms;
	static std::vector<Door> s_doors;
	static std::vector<Window> s_windows;
	static std::vector<Light> s_lights;	
	static std::vector<EntityStatic> s_staticEntities;

	static std::vector<BulletDecal> s_bulletDecals;
	static std::vector<BloodDecal> s_bloodDecals;
	static std::vector<VolumetricBloodSplatter> s_volumetricBloodSplatters;
	static std::vector<BulletCasing> s_bulletCasings;
	static std::vector<Bullet> s_bulletsToProcess;

	static void Clear();
	static void Update(float deltaTime);
	static void ProcessUnprocessedBullets();

	static void SpawnBullet(glm::vec3 position, glm::vec3 unitDir, float force, float damage, int parentPlayerIndex, glm::vec3 parentCamerasRotation);

	static void CreateVolumetricBlood(glm::vec3 position, glm::vec3 rotation, glm::vec3 front);
	static void DetermineIfLightNeedsShadowmapUpdate(Light& light);

	static void DrawInstancedGeometry(Shader* shader);
	static void DrawInstancedBulletDecals(Shader* shader);
	static void DrawInstancedBloodDecals(Shader* shader);

	static Player* GetPlayerPtrFromIndex(int index);

	static bool s_splitScreen;

	static std::vector<Controller> s_controllers;
		
	static unsigned int s_instancingBuffer;
	static unsigned int s_remainingBloodDecalsAllowedThisFrame;

	static void AddLight(glm::vec3 position, glm::vec3 color, float radius, float strength, float magic, int modelType);

public: // Inlines
	static void FlagEnvMapsForReRendering() { s_renderEnvMaps = true; }
	//static void UnflagEnvMapsForReRendering() { s_renderEnvMaps = false; }
	//static bool EnvMapsNeedReRendering() { return s_renderEnvMaps; }

	static bool s_renderEnvMaps;
	// internals
private:
	static int s_volumetricBloodObjectsSpawnedThisFrame;
};