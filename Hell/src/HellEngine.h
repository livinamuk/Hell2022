#pragma once
#include "Header.h"
#include "Core/Scene.h"
#include "Core/CoreGL.h"
#include "Core/Input.h"
#include "Core/Camera.h"
#include "Core/Entity.h"
#include "Helpers/Util.h"
#include "Helpers/FileImporter.h"
#include "Renderer/Decal.h"
#include "Renderer/Model.h"
#include "Renderer/Texture.h"
#include "Renderer/TextBlitter.h"
#include "Renderer/Renderer.h"

#include "Physics/PhysX.h"
#include "Helpers/AssetManager.h"
#include "Core/Player.h"

class HellEngine
{
public: // methods
	void Init();
	void Update(float deltaTime);

	void UpdateInput();

	void CheckForDebugKeyPresses();
	void Render();
	void ProcessCollisions();

public: // fields
	PhysX m_physx;
	Renderer m_renderer;
	Scene m_scene;
	//Camera m_camera_p1;
	//Camera m_camera_p2;
	std::unordered_map<std::string, Model> m_models;
	bool m_Splitscreen = true;


	Player m_player1;
	Player m_player2;

	int m_currentPlayer = 1;

	std::vector<int> m_controllers;

	//bool m_controller_1_connected = false;

	//Entity couchEntity;
};