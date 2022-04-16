#include "GameData.h"

Player GameData::s_player1;
Player GameData::s_player2;
std::vector<Room> GameData::s_rooms;
std::vector<Door> GameData::s_doors;
bool GameData::s_splitScreen = true;

void GameData::Clear()
{
	s_rooms.clear();
	s_doors.clear();
}

void GameData::Update(float deltaTime)
{
	for (Door& door : s_doors)
		door.Update();
}

