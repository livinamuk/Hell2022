#pragma once
#include "Header.h"

class RayCast 
{
public: // methods
	RayCast();
	void CastRay(glm::vec3 rayOrigin, glm::vec3 rayDirection, float rayLength);
	bool HitFound();

public: // fields
	std::string m_hitObjectName;
	glm::vec3 m_hitPosition;
	glm::vec3 m_surfaceNormal;
	glm::vec3 m_rayDirection;
	bool m_hitFound;
	void* m_hitActor;
	void* m_parent;
	PhysicsObjectType m_physicsObjectType;
};