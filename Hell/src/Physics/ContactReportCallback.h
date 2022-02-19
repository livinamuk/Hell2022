#pragma once

#ifdef _DEBUG 
#define DEBUG
#else
#define NDEBUG
#endif

#define PVD_HOST "127.0.0.1"
#include "PxPhysicsAPI.h"
#include "Header.h"

using namespace physx;

struct CollisionReport {
	PxRigidActor* rigidA;
	PxRigidActor* rigidB;
	EntityData* dataA;
	EntityData* dataB;
};

class ContactReportCallback : public PxSimulationEventCallback
{
public:
	static std::vector<CollisionReport> s_collisionReports;


	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) { PX_UNUSED(constraints); PX_UNUSED(count); }
	void onWake(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
	void onSleep(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
	void onTrigger(PxTriggerPair* pairs, PxU32 count) { PX_UNUSED(pairs); PX_UNUSED(count); }
	void onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32) {}

	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
	
};