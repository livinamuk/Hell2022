#include "Entity.h"
#include "Helpers/Util.h"


Entity::Entity()
{
}

Entity::Entity(Model* model, GLuint albedoTextureID)
{
	//std::cout << "entity has model: " << model->m_filePath << "\n";
	this->m_model = model;
	this->m_albedoTextureID = albedoTextureID;
	ConfigureDecalMap(1024, 1024);
}

void Entity::Update(float deltaTime)
{

}

void Entity::DrawEntity(Shader* shader)
{
	shader->setBool("u_hasDecalMap", true);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_albedoTextureID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_decalMapID);
	m_model->Draw(shader, m_transform.to_mat4());
	shader->setBool("u_hasDecalMap", false);
}

void Entity::UpdateCollisionObject()
{
	// return; // cunt
	PxScene* scene = PhysX::GetScene();
	PxPhysics* physics = PhysX::GetPhysics();
	PxMaterial* material = PhysX::GetDefaultMaterial();

	PxMeshScale scale(PxVec3(m_transform.scale.x, m_transform.scale.y, m_transform.scale.z), PxQuat(PxIdentity));
	PxTriangleMeshGeometry geom(m_model->m_triMesh, scale);

	PxTransform pose(Util::GlmMat4ToPxMat44(m_transform.to_mat4()));
	m_actor = PxCreateStatic(*physics, pose, geom, *material);
	scene->addActor(*m_actor);
}

/*
void Entity::RemoveCollisionObject()
{
	Physics::s_dynamicsWorld->removeCollisionObject(m_collisionObject);
}
*/
void Entity::ConfigureDecalMap(int width, int height)
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