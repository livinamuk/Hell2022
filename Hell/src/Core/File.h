#pragma once

//#include "House/House.h"
#include "Core/GameData.h"
#include "Helpers/Util.h"

#include "rapidjson/document.h"
#include <rapidjson/filereadstream.h>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <fstream>
#include <string>
#include <iostream>
#include "Helpers/AssetManager.h"

class File
{
public: // static fields
	//static std::vector<std::string> MapList;

public: // static methods
	//static void LoadMapNames();

	static void LoadMap(std::string filename);
	static void SaveMap(std::string filename);

	static void SaveVec2(rapidjson::Value* object, std::string elementName, glm::vec2 vector, rapidjson::Document::AllocatorType& allocator);
	static void SaveVec3(rapidjson::Value* object, std::string elementName, glm::vec3 vector, rapidjson::Document::AllocatorType& allocator);
	static void SaveString(rapidjson::Value* object, std::string elementName, std::string string, rapidjson::Document::AllocatorType& allocator);
	static void SaveBool(rapidjson::Value* object, std::string elementName, bool boolean, rapidjson::Document::AllocatorType& allocator);
	static void SaveFloat(rapidjson::Value* object, std::string elementName, float number, rapidjson::Document::AllocatorType& allocator);
	static void SaveInt(rapidjson::Value* object, std::string elementName, int number, rapidjson::Document::AllocatorType& allocator);

	static glm::vec2 ReadVec2(const rapidjson::Value& value, std::string name);
	static glm::vec3 ReadVec3(const rapidjson::Value& value, std::string name);
	static std::string ReadString(const rapidjson::Value& value, std::string name);
	static char* ReadText(const rapidjson::Value& value, std::string name);
	static bool ReadBool(const rapidjson::Value& value, std::string name);
	static float ReadFloat(const rapidjson::Value& value, std::string name);
	static int ReadInt(const rapidjson::Value& value, std::string name);
};
