#pragma once
#include "Header.h"
#include "Renderer/Model.h"
#include "Physics/PhysX.h"

class Entity
{
public: // methods

	Entity();
	Entity(Model* model, GLuint textureID);
	void Update(float deltaTime);
	void DrawEntity(Shader* shader);
	void UpdateCollisionObject();
//	void RemoveCollisionObject();
	void ConfigureDecalMap(int width, int height);

public:	// fields
	Transform m_transform;
	PxRigidStatic* m_actor;

	GLuint m_decalMapFBO;
	GLuint m_decalMapID;
	GLuint m_albedoTextureID;


private: // field
	Model* m_model;
};