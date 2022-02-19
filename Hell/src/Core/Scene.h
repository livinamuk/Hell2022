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
	void Reset();
	void NewRagdoll();
	void AddEntity(Model* model, Material* material, Transform transform);

public: // fields
//	SkinnedModel* nurseModel;
	std::vector<GameCharacter> m_gameCharacters;
	std::vector<EntityStatic> m_staticEntities;
	std::vector<BloodPool> m_bloodPools;


	//GameCharacter m_gameCharacterPlayer1;
	//GameCharacter m_gameCharacterPlayer2;

//	std::vector<Entity> m_staticEntities;
};