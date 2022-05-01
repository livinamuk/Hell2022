#pragma once
#include "Header.h"
#include "Core/Controller.h"
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
#include "Editor/Editor.h"


class HellEngine
{
public: // methods
	void Init();
	void Update(float deltaTime);

	void UpdateInput();

	void CheckForDebugKeyPresses();
	void Render();
	void ProcessCollisions();

	void CheckForControllers();

public: // fields
	PhysX m_physx;
	std::unordered_map<std::string, Model> m_models;

	int m_currentPlayer = 1;
	bool m_switchToPlayer2 = false;
};