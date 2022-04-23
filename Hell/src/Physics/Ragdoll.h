#pragma once

#include "rapidjson/document.h"
#include <rapidjson/filereadstream.h>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>

//#include "Header.h"
#include "Helpers/Util.h"

#include "PhysX.h"
#include "Audio/Audio.h"


struct RigidComponent
{
public:
	int ID;
	std::string name, correspondingJointName, shapeType;
	glm::mat4 restMatrix;
	glm::vec3 scaleAbsoluteVector;
	glm::vec3 boxExtents, offset;
	float capsuleLength, capsuleRadius;
	PxQuat rotation;
	float mass, friction, restitution, linearDamping, angularDamping, sleepThreshold;
	PxVec3 angularMass;
	PxRigidDynamic* pxRigidBody = nullptr;
};

struct JointComponent
{
public:
	std::string name;
	int parentID, childID;
	PxMat44 parentFrame, childFrame;
	PxD6Joint* pxD6 = nullptr;
	
	// Drive component
	float drive_angularDamping, drive_angularStiffness, drive_linearDampening, drive_linearStiffness;
	glm::mat4 target;
	bool drive_enabled;

	// Limit component
	float twist, swing1, swing2, limit_angularStiffness, limit_angularDampening, limit_linearStiffness, limit_linearDampening;

	glm::vec3 limit;
	bool joint_enabled;
};

struct DriveComponent
{
public:

};


class Ragdoll
{
public:
	Ragdoll();
	void RemovePhysicsObjects();
	std::vector<RigidComponent> m_rigidComponents;
	std::vector<JointComponent> m_jointComponents;
	std::vector<glm::mat4> m_animatedTransforms;
	std::vector<glm::mat4> m_animatedDebugTransforms_Animated;

	//bool m_hasHitGroundYet = false;


	//void* m_parent = nullptr;

	void BuildFromJsonFile(std::string filename, Transform spawnLocation, void* parent, PhysicsObjectType type);

	RigidComponent* GetRigidByName(std::string name);

	//void HitFloor();

private:
	std::string FindParentJointName(std::string query);
};

