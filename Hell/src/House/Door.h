#pragma once
#include "Header.h"
#include "Physics/PhysX.h"


class Door
{ 
public:
	enum class State { CLOSED, CLOSING, OPEN, OPENING};

	// Static shit
public:
	//static physx::PxShape* s_doorShape;
	//static physx::PxShape* s_frameShape;
	static void CreateShapes();

	// Member shit
public:
	Transform m_transform;
	Transform m_OffsetTransform;
	Transform m_physicalDoorOffsetTransform; // where the actual door model gets rendererd

	unsigned int m_parentIndexVertexA;
	unsigned int m_parentIndexVertexB;
	unsigned int m_parentRoomIndex;

	float m_swing = 0;
	bool m_closed = true;

	physx::PxRigidDynamic* m_rigid = nullptr;
	physx::PxRigidStatic* m_frameRigid;
	physx::PxD6Joint* m_joint;
	PxTransform m_restPose;
	PxShape* m_shape;

	Door::State m_state = State::CLOSING;


	Door();
	Door(glm::vec3 position);
	void Draw(Shader* shader);
	void DrawForEditor(Shader* shader, glm::vec3 planeColor);

	glm::vec3 GetVert1(glm::vec3 localOffset = glm::vec3(0));
	glm::vec3 GetVert2(glm::vec3 localOffset = glm::vec3(0));
	glm::vec3 GetVert3(glm::vec3 localOffset = glm::vec3(0));
	glm::vec3 GetVert4(glm::vec3 localOffset = glm::vec3(0));

	glm::vec3 GetParentVert1();
	glm::vec3 GetParentVert2();
	glm::vec3* GetPointerToParentVert1();
	glm::vec3* GetPointerToParentVert2();

	void Interact();
	void Update(float deltaTime);

	void CreateCollisionObject();
	void RemoveCollisionObject();

	glm::mat4 GetModelMatrix();
};