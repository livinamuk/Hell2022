#include "RayCast.h"
#include "PhysX.h"

RayCast::RayCast()
{

}

RayCast::RayCast(glm::vec3 rayOrigin, glm::vec3 rayDirection, float rayLength)
{
	CastRay(rayOrigin, rayDirection, rayLength);
}

void RayCast::CastRay(glm::vec3 rayOrigin, glm::vec3 rayDirection, float rayLength)
{
	PxScene* scene = PhysX::GetScene();
	PxVec3 origin = PxVec3(rayOrigin.x, rayOrigin.y, rayOrigin.z);
	PxVec3 unitDir = PxVec3(rayDirection.x, rayDirection.y, rayDirection.z);
	PxReal maxDistance = rayLength;
	PxRaycastBuffer hit;

	// [in] Define what parts of PxRaycastHit we're interested in
	const PxHitFlags outputFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
	// Only ray cast against objects with the GROUP_RAYCAST flag
	PxQueryFilterData filterData = PxQueryFilterData();
	filterData.data.word0 = GROUP_RAYCAST;

	// Cast the ray
	bool status = scene->raycast(origin, unitDir, maxDistance, hit, outputFlags, filterData);

	// Defaults
	m_hitObjectName = "NO_HIT";
	m_hitPosition = glm::vec3(0, 0, 0);
	m_surfaceNormal = glm::vec3(0, 0, 0); 
	m_rayDirection = rayDirection;
	m_hitFound = false;
	m_hitActor = nullptr;
	m_parent = nullptr;
	m_physicsObjectType = PhysicsObjectType::UNDEFINED;

	if (status)
	{
		if (hit.block.actor->getName()) {
			m_hitObjectName = hit.block.actor->getName();
		}
		else
			m_hitObjectName = "HIT OBJECT HAS NO ACTOR NAME";

		m_hitPosition = glm::vec3(hit.block.position.x, hit.block.position.y, hit.block.position.z);
		m_surfaceNormal = glm::vec3(hit.block.normal.x, hit.block.normal.y, hit.block.normal.z);
		m_hitFound = true;
		m_hitActor = hit.block.actor;

		EntityData* hitEntityData = (EntityData*)hit.block.actor->userData;		
		m_parent = hitEntityData->parent;
		m_physicsObjectType = hitEntityData->type;
	}
}

bool RayCast::HitFound()
{
	return m_hitFound;
}
