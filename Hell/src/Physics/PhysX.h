#pragma once

//#define NDEBUG

#ifdef _DEBUG 
	#define DEBUG
#else
	#define NDEBUG
#endif

#define PVD_HOST "127.0.0.1"
#include "PxPhysicsAPI.h"
#include "Header.h"
#include "ContactReportCallback.h"

struct ControlledActorDesc
{
	//ControlledActorDesc();

	PxControllerShapeType::Enum		mType;
	PxExtendedVec3					mPosition;
	float							mSlopeLimit;
	float							mContactOffset;
	float							mStepOffset;
	float							mInvisibleWallHeight;
	float							mMaxJumpHeight;
	float							mRadius;
	float							mHeight;
	float							mCrouchHeight;
	float							mProxyDensity;
	float							mProxyScale;
	float							mVolumeGrowth;
	PxUserControllerHitReport* mReportCallback;
	PxControllerBehaviorCallback* mBehaviorCallback;
};

class PhysX
{


public: // methods
	void Init();
	void Shutdown(); 
	void StepPhysics();

	static PxController* CreateCharacterController(Transform transform);
	static PxRigidDynamic* CreateBox(Transform transform, glm::vec3 velocity);
	//static void ProcessCollision(CollisionReport collision);

	PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity);
	
	void UpdateDrives();

	PxTriangleMesh* CreateBV33TriangleMesh(PxU32 numVertices, const PxVec3* vertices, PxU32 numTriangles, const PxU32* indices, bool skipMeshCleanup, bool skipEdgeData, bool inserted, bool cookingPerformance, bool meshSizePerfTradeoff);
	void SetupCommonCookingParams(PxCookingParams& params, bool skipMeshCleanup, bool skipEdgeData);

public: // field
	PxFoundation* mFoundation;
	PxPvd* mPvd;
	PxPhysics* mPhysics;
	PxDefaultCpuDispatcher* mDispatcher = NULL;
	PxScene* mScene = NULL;
	PxCooking* mCooking = NULL; 

	std::vector<PxRigidDynamic*> m_rigids;

	ContactReportCallback gContactReportCallback;
	static std::vector<PxVec3> gContactPositions;
	static std::vector<PxVec3> gContactImpulses;	
	
public: // static functions
	static PxPhysics* GetPhysics();
	static PxScene* GetScene();
	static PxMaterial* GetDefaultMaterial();
	static PxCooking* GetCooking();

	static PxShape* GetShapeFromPxRigidDynamic(PxRigidDynamic* rigid);
	static void EnableRayCastingForShape(PxShape* shape);
	static void DisableRayCastingForShape(PxShape* shape);

public: // static variables
	static PhysX* p_PhysX;
	static PxCooking* s_Cooking;
	static PxControllerManager* s_characterControllerManager;


};





