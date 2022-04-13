#pragma once
#include "Header.h"
#include "Core/Player.h"
#include "Helpers/AssetManager.h"
#include "House/Room.h"
#include "House/Door.h"

#include "rapidjson/document.h"
#include <rapidjson/filereadstream.h>

class GameData {
public:
	static Player s_player1;
	static Player s_player2;
	static std::vector<Room> s_rooms;
	static std::vector<Door> s_doors;

	static void LoadMap(std::string path);
	static void SaveMap(std::string path);
};