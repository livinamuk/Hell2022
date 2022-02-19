#pragma once
#include "Header.h"
#include "Renderer/Material.h"
#include "Renderer/Model.h"
#include "Physics/PhysX.h"

class EntityStatic
{
public: // methods
	EntityStatic();
	EntityStatic(Model* model, Material* material, Transform transform);
	void DrawEntity(Shader* shader);
	void SetPosition(glm::vec3 position);
	//	void RemoveCollisionObject();
	void ConfigureDecalMap(int width, int height);

public:	// fields
	physx::PxRigidStatic* m_actor;
	GLuint m_decalMapFBO;
	GLuint m_decalMapID;
	Material* m_material;
	Model* m_model;

private: // methods
	void UpdateCollisionObject();

private: // field
	Transform m_transform;
};
