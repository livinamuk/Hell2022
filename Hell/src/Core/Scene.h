#pragma once
#include "Header.h"
#include "Core/GameCharacter.h"
#include "Core/EntityStatic.h"
#include "Animation/SkinnedModel.h"
#include "Core/Player.h"
#include "Effects/BloodPool.h"

class Scene
{
public: // methods
	static void Reset();
	static void NewRagdoll();
	static void AddEntity(Model* model, Material* material, Transform transform);

public: // variables
	static std::vector<GameCharacter> s_gameCharacters;
	static std::vector<EntityStatic> s_staticEntities;
	static std::vector<BloodPool> s_bloodPools;
};