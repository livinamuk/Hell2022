#pragma once
#include "Header.h"
#include "Physics/PhysX.h"

class BulletCasing
{
public:
	enum class CasingType { GLOCK_CASING, SHOTGUN_SHELL };

public:
	// Methods
	BulletCasing();
	BulletCasing(Transform transform, glm::vec3 rotation, CasingType type);
	void Update(float deltatime);
	//void Draw(Shader* shader);  instanced now inside the class GameData. put this somewhere better

	// Fields
	glm::mat4 m_modelMatrix = glm::mat4(1); 
	PxRigidDynamic* m_rigidDynamic;
	CasingType m_type;
	bool m_hitWall = false;
	bool m_hitFloor = false;
	bool m_playedWallSound = false;
	bool m_playedFloorSound = false;
	float m_lifetime = 0;
};