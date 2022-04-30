#include "BulletCasing.h"
#include "Helpers/Util.h"
#include "Helpers/AssetManager.h"
#include "Core/Input.h"

BulletCasing::BulletCasing(Transform transform, glm::vec3 velocity, CasingType type)
{
	m_type = type;

	PxShape* shape;
	const PxMaterial* gMaterial = PhysX::GetDefaultMaterial();
	glm::vec3 boxExtents = glm::vec3(0.01, 0.01, 0.1);
	const PxBoxGeometry extents = PxBoxGeometry(boxExtents.x * 0.5, boxExtents.y * 0.5, boxExtents.z * 0.5);
	PxPhysics* physx = PhysX::GetPhysics();
	shape = physx->createShape(extents, *gMaterial);

	PxMat44 spawnMatrix = Util::TransformToPxMaQt44(transform);

	m_rigidDynamic = physx->createRigidDynamic(PxTransform(spawnMatrix));
	m_rigidDynamic->attachShape(*shape);
	m_rigidDynamic->setSolverIterationCounts(8, 1);
	m_rigidDynamic->setName("BULLET_CASING");
	// userdata is now set per frame.
	//m_rigidDynamic->userData = new EntityData(PhysicsObjectType::SHELL_PROJECTILE, this);
		

	PxFilterData filterData;
	filterData.word1 = PhysX::CollisionGroup::MISC_OBSTACLE;
	filterData.word2 = PhysX::CollisionGroup::MISC_OBSTACLE;
	shape->setQueryFilterData(filterData);
	shape->setSimulationFilterData(filterData);
	shape->setQueryFilterData(filterData);


	PxScene* scene = PhysX::GetScene();
	scene->addActor(*m_rigidDynamic);
	float mass = 1;
	PxRigidBodyExt::setMassAndUpdateInertia(*m_rigidDynamic, mass);

	m_rigidDynamic->setLinearVelocity(PxVec3(velocity.x, velocity.y, velocity.z));

	shape->release();
}

void BulletCasing::Update(float deltatime)
{
	m_lifetime += deltatime;

	if (m_hitFloor && !m_playedFloorSound) {
		m_playedFloorSound = true;
		if (m_type == CasingType::GLOCK_CASING)
			Audio::PlayAudio("BulletCasingBounce.wav", 0.2f);
		if (m_type == CasingType::SHOTGUN_SHELL)
			Audio::PlayAudio("ShellFloorBounce.wav", 0.4f);
	}

	// Get model matrix from physx sim 
	if (m_rigidDynamic)
	{
		if (m_type == BulletCasing::CasingType::GLOCK_CASING) {
			Transform localOffset;
			localOffset.rotation.y = -HELL_PI / 2;
			localOffset.scale = glm::vec3(3.0f);
			m_modelMatrix = Util::PxMat44ToGlmMat4(m_rigidDynamic->getGlobalPose()) * localOffset.to_mat4();
		}
		else if (m_type == BulletCasing::CasingType::SHOTGUN_SHELL) {
			Transform localOffset;
			localOffset.scale = glm::vec3(2.0f);
			localOffset.rotation.x = HELL_PI / 2;
			m_modelMatrix = Util::PxMat44ToGlmMat4(m_rigidDynamic->getGlobalPose()) * localOffset.to_mat4();
		}
	}

	// Remove from physics world after 4 seconds
	if (m_lifetime > 4 && m_rigidDynamic) {
		delete m_rigidDynamic->userData;
		m_rigidDynamic->release();
		m_rigidDynamic = nullptr;
	}
}

/*void BulletCasing::Draw(Shader* shader)
{
	Transform localOffset;

	glm::mat4 modelMatrix = Util::PxMat44ToGlmMat4(m_rigidDynamic->getGlobalPose());

	if (m_type == CasingType::GLOCK_CASING) {
		localOffset.rotation.y = -HELL_PI / 2;
		localOffset.scale = glm::vec3(3.0f);
		AssetManager::GetMaterialPtr("BulletCasing")->Bind();
		AssetManager::GetModelPtr("BulletCasing")->Draw(shader, modelMatrix * localOffset.to_mat4());
	}

	else if (m_type == CasingType::SHOTGUN_SHELL) {	
		localOffset.scale = glm::vec3(2.0f);
		localOffset.rotation.x = HELL_PI / 2;
		AssetManager::GetMaterialPtr("Shell")->Bind();
		AssetManager::GetModelPtr("Shell")->Draw(shader, modelMatrix * localOffset.to_mat4());
	}
}*/