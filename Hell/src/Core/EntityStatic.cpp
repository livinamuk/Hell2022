#include "EntityStatic.h"
#include "Helpers/Util.h"

EntityStatic::EntityStatic()
{
}

EntityStatic::EntityStatic(Model* model, Material* material, Transform transform)
{
	this->m_model = model;
	this->m_material = material;
	this->m_transform = transform;

	//ConfigureDecalMap(1024, 1024);

	if (m_model->m_hasCollisionMesh) {
		UpdateCollisionObject();
		//std::cout << model->m_filePath <<  " updated collision object\n";
	}
	else {
		//std::cout << model->m_filePath << " has no collision object\n";
	}
}

void EntityStatic::DrawEntity(Shader* shader)
{
	shader->setBool("u_hasDecalMap", true);
	m_material->Bind();
	m_model->Draw(shader, m_transform.to_mat4());
	shader->setBool("u_hasDecalMap", false);
}

void EntityStatic::SetPosition(glm::vec3 position)
{
	m_transform.position = position;

	physx::PxTransform pose(Util::GlmMat4ToPxMat44(m_transform.to_mat4()));
	m_actor->setGlobalPose(pose);
}

void EntityStatic::UpdateCollisionObject()
{
	physx::PxScene* scene = PhysX::GetScene();
	physx::PxPhysics* physics = PhysX::GetPhysics();
	physx::PxMaterial* material = PhysX::GetDefaultMaterial();

	physx::PxMeshScale scale(physx::PxVec3(m_transform.scale.x, m_transform.scale.y, m_transform.scale.z), physx::PxQuat(physx::PxIdentity));
	//physx::PxMeshScale scale(physx::PxVec3(1, 1, 1), physx::PxQuat(physx::PxIdentity));
	physx::PxTriangleMeshGeometry geom(m_model->m_triMesh, scale);

	physx::PxTransform pose(Util::GlmMat4ToPxMat44(m_transform.to_mat4()));
	m_actor = PxCreateStatic(*physics, pose, geom, *material);
	
	scene->addActor(*m_actor);

	m_actor->setName("Triangle Mesh");
	m_actor->userData = new EntityData(PhysicsObjectType::MISC_MESH, this);

	// enable raycasting for this shape
	PxShape* shape;
	m_actor->getShapes(&shape, 1);
	PhysX::EnableRayCastingForShape(shape);

	PxFilterData filterData;
	filterData.word1 = PhysX::CollisionGroup::MISC_OBSTACLE;
	shape->setQueryFilterData(filterData);
	shape->setSimulationFilterData(filterData);
}

/*
void Entity::RemoveCollisionObject()
{
	Physics::s_dynamicsWorld->removeCollisionObject(m_collisionObject);
}
*/
void EntityStatic::ConfigureDecalMap(int width, int height)
{
	glGenFramebuffers(1, &m_decalMapFBO);
	glGenTextures(1, &m_decalMapID);

	glBindFramebuffer(GL_FRAMEBUFFER, m_decalMapFBO);

	glBindTexture(GL_TEXTURE_2D, m_decalMapID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_decalMapID, 0);
	//	glGenerateMipmap(GL_TEXTURE_2D); // dunno about this, is it neccessary??

	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);

	// Clear it to test it works
	float clearcolor[] = { 0, 0, 1, 1 };
	glClearTexImage(m_decalMapID, 0, GL_RGBA, GL_FLOAT, clearcolor);

	glClearColor(1, 1, 0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
