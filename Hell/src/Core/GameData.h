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


class GameData {
public:
	static Player s_player1;
	static Player s_player2;
	static std::vector<Room> s_rooms;
	static std::vector<Door> s_doors;
	static std::vector<Light> s_lights;	
	static std::vector<EntityStatic> s_staticEntities;
	static std::vector<BulletDecal> s_bulletDecals;

	static void Clear();
	static void Update(float deltaTime);

	static bool s_splitScreen;
	
};