#include "PhysX.h"
#include "Helpers/Util.h"
#include "extensions/PxDefaultAllocator.h"


class UserErrorCallback : public PxErrorCallback
{
public:
	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file,
		int line)
	{
        std::cout << file << " line " << line << ": " << message << "\n";
        std::cout << "\n";
	}
}gErrorCallback;


PxCooking* PhysX::s_Cooking;
PhysX* PhysX::p_PhysX;
//PxMaterial s_defaultMaterial;

using namespace physx;

PxDefaultAllocator		gAllocator;
//PxDefaultErrorCallback	gErrorCallback;


PxReal chainZ = 10.0f;

//ContactReportCallback PhysX::gContactReportCallback;
std::vector<PxVec3> PhysX::gContactPositions;
std::vector<PxVec3> PhysX::gContactImpulses;
PxControllerManager* PhysX::s_characterControllerManager;





PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	PX_UNUSED(attributes0);
	PX_UNUSED(attributes1);
	//PX_UNUSED(filterData0);
	//PX_UNUSED(filterData1);
	PX_UNUSED(constantBlockSize);
	PX_UNUSED(constantBlock);

	//if (!(filterData0.word1 & filterData1.word1)) {
	if (!(filterData0.word1 & filterData1.word1) && !(filterData0.word2 & filterData1.word2)) {
		return PxFilterFlag::eKILL;
	}

	pairFlags = PxPairFlag::eSOLVE_CONTACT
		| PxPairFlag::eDETECT_DISCRETE_CONTACT
		| PxPairFlag::eNOTIFY_TOUCH_FOUND
		| PxPairFlag::eNOTIFY_TOUCH_PERSISTS
		| PxPairFlag::eNOTIFY_CONTACT_POINTS
		| PxPairFlag::eDETECT_CCD_CONTACT;
	return PxFilterFlag::eDEFAULT;
}

PxFilterFlags PhysicsMainFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1,
	PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
	//if (!(filterData0.word1 & filterData1.word2) || !(filterData1.word1 & filterData0.word2)) {
//	if (!(filterData0.word2 & filterData1.word1) || !(filterData0.word1 & filterData1.word2)) {
		//	return PxFilterFlag::eKILL;
	//}
	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eTRIGGER_DEFAULT | PxPairFlag::eNOTIFY_CONTACT_POINTS;
	return PxFilterFlag::eDEFAULT;
}

void PhysX::Init()
{

    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
    mPvd = PxCreatePvd(*mFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(), true, mPvd);
    PxInitExtensions(*mPhysics, mPvd);

	PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	//sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	//sceneDesc.filterShader = PhysicsMainFilterShader;
	sceneDesc.filterShader = contactReportFilterShader;
    sceneDesc.simulationEventCallback = &gContactReportCallback;
    
    mDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = mDispatcher;
   // u were using this, but switched for above //  sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    mScene = mPhysics->createScene(sceneDesc);


	s_characterControllerManager = PxCreateControllerManager(*mScene);
	

    PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }


    PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, PxPlane(0, 1, 0, 0), *GetDefaultMaterial());
    groundPlane->setName("GROUND");
    groundPlane->userData = new EntityData(PhysicsObjectType::FLOOR, nullptr);


    // enable raycasting for this shape
    PxShape* shape;
    groundPlane->getShapes(&shape, 1);
    EnableRayCastingForShape(shape);


    // word 1 is this shapes group
    // word 2 is shapes it collides with


	PxFilterData filterData;
	filterData.word0 = GROUP_RAYCAST;
	filterData.word1 = PhysX::CollisionGroup::MISC_OBSTACLE;
	filterData.word2 = PhysX::CollisionGroup::RAGDOLL_GROUP;
	shape->setQueryFilterData(filterData);
	shape->setSimulationFilterData(filterData); // sim is for ragz

    mScene->addActor(*groundPlane);

    mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, PxCookingParams(PxTolerancesScale()));
    s_Cooking = mCooking;


    return;





   /* static PxDefaultErrorCallback gDefaultErrorCallback;
    static PxDefaultAllocator gDefaultAllocatorCallback;
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);

    mPvd = PxCreatePvd(*mFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
    mPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, PxTolerancesScale(), true, mPvd);

    PxInitExtensions(*mPhysics, mPvd);

    PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    mDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = mDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    mScene = mPhysics->createScene(sceneDesc);


#ifdef _DEBUG 
    mScene->getScenePvdClient()->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
#endif

    PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
    auto mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxRigidStatic* groundPlane = PxCreatePlane(*mPhysics, PxPlane(0, 1, 0, 0), *mMaterial);
    mScene->addActor(*groundPlane);


    bool createCapsule = false;
    if (createCapsule)
    {
        float halfExtent = 5;
        float radius = 0.5;
        auto gMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
        PxShape* shape = mPhysics->createShape(PxCapsuleGeometry(radius, halfExtent), *gMaterial);


        PxTransform t = PxTransform(PxVec3(0, 2, 4));
        PxTransform localTm(PxVec3(0, 0, 0));

        PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
        body->attachShape(*shape);
        PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
        mScene->addActor(*body);


        m_rigids.push_back(body);
        shape->release();
    } 
    
    bool createSphere = false;
    if (createSphere)
    {
        float radius = 0.5;
        auto gMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
        PxShape* shape = mPhysics->createShape(PxSphereGeometry(radius), *gMaterial);


        PxTransform t = PxTransform(PxVec3(0, 5, 6));
        PxTransform localTm(PxVec3(0, 0, 0));

        PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
        body->attachShape(*shape);
        PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
        mScene->addActor(*body);


        m_rigids.push_back(body);
        shape->release();
    }
  
    PxReal stackZ = 10.0f;
    float halfExtent = 0.5f;
    float size = 10;
    auto gMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    PxTransform t = PxTransform(PxVec3(0, 0, stackZ -= 10.0f));

    bool createCubes = false;
    if (createCubes)
    {
        PxShape* shape = mPhysics->createShape(PxBoxGeometry(halfExtent * 0.5, halfExtent, halfExtent * 2), *gMaterial);
        for (PxU32 i = 0; i < size; i++)
        {
            for (PxU32 j = 0; j < size - i; j++)
            {
                PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
                PxRigidDynamic* body = mPhysics->createRigidDynamic(t.transform(localTm));
                body->attachShape(*shape);
                PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
                mScene->addActor(*body);

                m_rigids.push_back(body);
            }
        }
        shape->release();
    }

    */


}

void PhysX::Shutdown()
{
    mPhysics->release(); 
    mFoundation->release();
}

void PhysX::StepPhysics(float deltaTime)
{
   /* PxScene* scene;
    const PxPhysics& physX = PxGetPhysics();
    PxGetPhysics().getScenes(&scene, 1);

    PxPhysics::get

    physX.get<SolverComponent>(sceneEntity);

    PxRigidDynamic::getSolverIterationCounts

    mSolver.get
    const auto solver = g.registry.get<SolverComponent>(sceneEntity);
    const float stepsize = time.delta() * solver.timeMultiplier / solver.substeps;
    const double fraction = 1.0f / solver.substeps;

    for (unsigned int i = 1; i <= solver.substeps; i++) {
        updateRigidsSubstep(sceneEntity, fraction * i);

        pxscene->simulate(stepsize);
        pxscene->fetchResults(true);*/
    

    //mScene->simulate(1.0f / 60.0f);

    //return;

    float maxSimulateTime = (1.0f / 60.0f) * 4.0f;

    mScene->simulate(std::min(deltaTime, maxSimulateTime));
    mScene->fetchResults(true);
}



#define CONTACT_OFFSET			0.01f
#define STEP_OFFSET				0.05f
#define SLOPE_LIMIT				0.0f
#define INVISIBLE_WALLS_HEIGHT	0.0f
#define MAX_JUMP_HEIGHT			0.0f

PxController* PhysX::CreateCharacterController(Transform transform)
{
	glm::vec3 pos = transform.position;

	PxMaterial* mat = GetDefaultMaterial();
	PxCapsuleControllerDesc* desc = new PxCapsuleControllerDesc;
	desc->setToDefault();
	desc->height = 0.3;
	desc->radius = 0.1;
	desc->position = PxExtendedVec3(pos.x, pos.y + 0.4, pos.z);
	desc->material = mat; 
    desc->stepOffset = 0.35;


    //desc->

   // desc->upDirection = PxVec3(0, 1, 0);

   // desc->

   // descmReportCallback = nullptr;// this;
	//desc.mBehaviorCallback = nullptr;// this;
	std::printf("VALID: %d \n", desc->isValid());

	PxController* c = s_characterControllerManager->createController(*desc);
	if (!c)
	{
        std::printf("Failed to instance a controller\n");
	}

	if (c)
	{

		//chr->userData = c;
		//chr->moving.connect(sigc::mem_fun(this, &PhysicsSystemPhysX::TestCharacterMove));
        std::printf("PhysX validated an actor's character controller\n");
	}
    EntityData* userData = new EntityData(PhysicsObjectType::UNDEFINED, nullptr);
    c->setUserData(userData);

    return c;
  /*  static const float gScaleFactor = 1.5f;
    static const float gStandingSize = 1.0f * gScaleFactor;
    static const float gCrouchingSize = 0.25f * gScaleFactor;
    static const float gControllerRadius = 0.3f * gScaleFactor;

    ControlledActorDesc desc;
    desc.mType = PxControllerShapeType::eCAPSULE;
    desc.mPosition = PxExtendedVec3(0, 5, 0);
    desc.mSlopeLimit = SLOPE_LIMIT;
    desc.mContactOffset = CONTACT_OFFSET;
    desc.mStepOffset = STEP_OFFSET;
    desc.mInvisibleWallHeight = INVISIBLE_WALLS_HEIGHT;
    desc.mMaxJumpHeight = MAX_JUMP_HEIGHT;
    desc.mRadius = gControllerRadius;
    desc.mHeight = gStandingSize;
    desc.mCrouchHeight = gCrouchingSize;
    desc.mReportCallback = nullptr;// this;
    desc.mBehaviorCallback = nullptr;// this;

    const float radius = desc.mRadius;
    float height = desc.mHeight;
    float crouchHeight = desc.mCrouchHeight;

    PxControllerDesc* cDesc;
    PxCapsuleControllerDesc capsuleDesc;

    PX_ASSERT(desc.mType == PxControllerShapeType::eCAPSULE);
    capsuleDesc.height = height;
    capsuleDesc.radius = radius;
    capsuleDesc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
    cDesc = &capsuleDesc;

    cDesc->density = desc.mProxyDensity;
    cDesc->scaleCoeff = desc.mProxyScale;
    cDesc->material = GetDefaultMaterial();
    cDesc->position = desc.mPosition;
    cDesc->slopeLimit = desc.mSlopeLimit;
    cDesc->contactOffset = desc.mContactOffset;
    cDesc->stepOffset = desc.mStepOffset;
    cDesc->invisibleWallHeight = desc.mInvisibleWallHeight;
    cDesc->maxJumpHeight = desc.mMaxJumpHeight;
    //	cDesc->nonWalkableMode		= PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
    cDesc->reportCallback = desc.mReportCallback;
    cDesc->behaviorCallback = desc.mBehaviorCallback;
    cDesc->volumeGrowth = desc.mVolumeGrowth;

    //mType = desc.mType;
    //mInitialPosition = desc.mPosition;
    //mStandingSize = height;
    //mCrouchingSize = crouchHeight;
    //mControllerRadius = radius;

    PxController* ctrl = static_cast<PxBoxController*>(s_characterControllerManager->createController(*cDesc));
    PX_ASSERT(ctrl);

    std::cout << "\n\nhello\n\n";
    std::cout << "Controller count: " << s_characterControllerManager->getNbControllers() << "\n";

    return ctrl;*/


   /* PxControllerDesc* cDesc;

    PxCapsuleControllerDesc desc;
    //desc.setToDefault();
    desc.radius = 1;
    desc.height = 0.5f;
    desc.upDirection = PxVec3(0, 1, 0);
    //desc.slopeLimit = cosf(PxMath::degToRad(45.0f)); 
    desc.slopeLimit = 0.785398;
    desc.stepOffset = 5.0;
    desc.position.x = transform.position.x;
    desc.position.y = 5;
    desc.position.z = transform.position.z;
    desc.material = GetDefaultMaterial();
    desc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
    desc.contactOffset = 0.1;
    desc.density = 10.0;
    desc.invisibleWallHeight = 0.0;
    desc.maxJumpHeight = 2;
    desc.nonWalkableMode = PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
    desc.scaleCoeff = 0.8;
    desc.volumeGrowth = 1.5f;
    desc.behaviorCallback = NULL;
    desc.reportCallback = NULL;
//desc.callback = NULL;


    PxController* ctrl = static_cast<PxBoxController*>(s_characterControllerManager->createController(desc));
    PX_ASSERT(ctrl);

 //   PxController* c = s_characterControllerManager->createController(cDesc);
    std::cout << "Created character controller\n";
    std::cout << "Controller count: " << s_characterControllerManager->getNbControllers() << "\n";
    
    return ctrl;*/
}

PxRigidDynamic* PhysX::CreateBox(Transform transform, glm::vec3 velocity)
{
    PxShape* shape;
    const PxMaterial* gMaterial = GetDefaultMaterial();

    glm::vec3 boxExtents = glm::vec3(0.01, 0.01, 0.1);

    const PxBoxGeometry extents = PxBoxGeometry(boxExtents.x * 0.5, boxExtents.y * 0.5, boxExtents.z * 0.5);

    PxPhysics* physx = PhysX::GetPhysics();

    shape = physx->createShape(extents, *gMaterial);
   

  //  PxTransform offsetTranslation = PxTransform(PxVec3(rigid.offset.x, rigid.offset.y, rigid.offset.z));
 //  PxTransform offsetRotation = PxTransform(rigid.rotation);

  //  shape->setLocalPose(offsetTranslation.transform(offsetRotation));

    PxMat44 spawnMatrix = Util::GlmMat4ToPxMat44(transform.to_mat4());

    PxRigidDynamic* rigidDynamic = physx->createRigidDynamic(PxTransform(spawnMatrix));
    rigidDynamic->attachShape(*shape);
    rigidDynamic->setSolverIterationCounts(8, 1);
    rigidDynamic->setName("CASING");
    rigidDynamic->userData = nullptr;

    PxScene* scene = GetScene();
    scene->addActor(*rigidDynamic);
    float mass = 1;
    PxRigidBodyExt::setMassAndUpdateInertia(*rigidDynamic, mass);

    //	std::cout << rigid.correspondingJointName << '\n';

   // PhysX::EnableRayCastingForShape(shape);


    shape->release();

   // dynamic->setAngularDamping(0.5f);
   /// dynamic->setLinearVelocity(velocity);
   // mScene->addActor(*dynamic);
    return rigidDynamic;
}




void PhysX::UpdateDrives() 
{
/*    for (auto entity : g.registry.view<SceneComponent, DriveComponent, PxD6Joint*>()) {
        if (g.registry.get<PxD6Joint*>(entity) == nullptr) continue;

        // Update only rigids in this scene
        const auto scene = g.registry.get<SceneComponent>(entity);
        if (scene.entity != sceneEntity) continue;

        const auto drive = g.registry.get<DriveComponent>(entity);
        const auto joint = g.registry.get<JointComponent>(entity);

        // We need this for our hack below, in inverting the child rest
        if (!g.registry.valid(joint.child)) {
            Warning() << "Joint is child-less, this is a bug\n";
            continue;
        }

        auto* pxjoint = g.registry.get<PxD6Joint*>(entity);
        if (drive.enabled) {
            PxD6JointDrive linearDrive{
                drive.linearStiffness,
                drive.linearDamping,

                FLT_MAX,   // Maximum force; ignored
                drive.acceleration,
            };

            pxjoint->setDrive(PxD6Drive::eX, linearDrive);
            pxjoint->setDrive(PxD6Drive::eY, linearDrive);
            pxjoint->setDrive(PxD6Drive::eZ, linearDrive);

            PxD6JointDrive angularDrive{
                drive.angularStiffness,
                drive.angularDamping,

                // Internal defaults
                FLT_MAX,
                drive.acceleration,
            };

            pxjoint->setDrive(PxD6Drive::eTWIST, angularDrive);
            pxjoint->setDrive(PxD6Drive::eSWING, angularDrive);
            pxjoint->setDrive(PxD6Drive::eSLERP, angularDrive);

            // Update target
            //
            // NOTE: Allow for changes to be made to both parent
            // and child frames, without affecting the drive target
            //
            // TODO: Unravel this. We currently can't edit the child anchorpoint
            MMatrix mat = joint.childFrame
                * drive.target
                * joint.parentFrame.inverse();
            // Take negative scale into account
            const auto parentScale = g.registry.get<ScaleComponent>(joint.parent);
            if (parentScale.value.x < 0 || parentScale.value.y < 0 || parentScale.value.z < 0) {
                mat = parentScale.matrix * mat;

                const MTransformationMatrix matTm{ mat };
                const MQuaternion quat = matTm.rotation();
                MTransformationMatrix finalTm{};
                finalTm.setTranslation(matTm.getTranslation(MSpace::kTransform), MSpace::kTransform);
                finalTm.setRotationQuaternion(quat.x, quat.y, quat.z, quat.w);

                mat = finalTm.asMatrix();
            }

            const PxTransform pxtransform{ toPxMat44(mat) };
            pxjoint->setDrivePosition(pxtransform);
        }

        else {
            static const PxD6JointDrive defaultDrive{ 0.0f, 0.0f, 0.0f, false };
            pxjoint->setDrive(PxD6Drive::eX, defaultDrive);
            pxjoint->setDrive(PxD6Drive::eY, defaultDrive);
            pxjoint->setDrive(PxD6Drive::eZ, defaultDrive);
            pxjoint->setDrive(PxD6Drive::eTWIST, defaultDrive);
            pxjoint->setDrive(PxD6Drive::eSWING, defaultDrive);
            pxjoint->setDrive(PxD6Drive::eSLERP, defaultDrive);
        }
    }*/
}

PxPhysics* PhysX::GetPhysics()
{
    return &PxGetPhysics();
}

PxScene* PhysX::GetScene()
{
    PxScene* scene;
    PxGetPhysics().getScenes(&scene, 1);
    return scene;
}

PxMaterial* PhysX::GetDefaultMaterial()
{
    static PxMaterial* defaultMaterial = nullptr;

    if (defaultMaterial == nullptr)
        defaultMaterial = GetPhysics()->createMaterial(0.5f, 0.5f, 0.6f);
    
    return defaultMaterial;
}

PxCooking* PhysX::GetCooking() {
return s_Cooking;
}


PxShape* PhysX::GetShapeFromPxRigidDynamic(PxRigidDynamic* rigid)
{
    if (rigid) {
        PxShape* shape;
        rigid->getShapes(&shape, 1);
        return shape;
    }
    else
        return nullptr;
}

void PhysX::EnableRayCastingForShape(PxShape* shape)
{
   // return;
    PxFilterData filterData = shape->getQueryFilterData();
	filterData.word0 = GROUP_RAYCAST;
	shape->setQueryFilterData(filterData);
}

void PhysX::DisableRayCastingForShape(PxShape* shape)
{
//	return;
    PxFilterData filterData = shape->getQueryFilterData();
    filterData.word0 = 0;
    shape->setQueryFilterData(filterData);
}


PxRigidDynamic* PhysX::createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity = PxVec3(0))
{
    PxRigidDynamic* dynamic = PxCreateDynamic(*mPhysics, t, geometry, *GetDefaultMaterial(), 10.0f);
    dynamic->setAngularDamping(0.5f);
    dynamic->setLinearVelocity(velocity);
    mScene->addActor(*dynamic);
    return dynamic;
}



PxTriangleMesh* PhysX::CreateBV33TriangleMesh(PxU32 numVertices, const PxVec3* vertices, PxU32 numTriangles, const PxU32* indices,  bool skipMeshCleanup, bool skipEdgeData, bool inserted, bool cookingPerformance, bool meshSizePerfTradeoff)
{
    //std::cout << "numVertices: " << numVertices << "\n";
    //std::cout << "numTriangles: " << numTriangles << "\n";

    PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = numVertices;
    meshDesc.points.data = vertices;
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.triangles.count = numTriangles;
    meshDesc.triangles.data = indices;
    meshDesc.triangles.stride = 3 * sizeof(PxU32);

    PxDefaultMemoryOutputStream writeBuffer;
    PxTriangleMeshCookingResult::Enum result;
    bool status = mCooking->cookTriangleMesh(meshDesc, writeBuffer);
    /*if (!status)
        std::cout << "FUCKED UP\n";
    else
        std::cout << "MESHED COOKED SUCCESSFULLY\n";*/

    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    
    return mPhysics->createTriangleMesh(readBuffer);
}

void PhysX::SetupCommonCookingParams(PxCookingParams& params, bool skipMeshCleanup, bool skipEdgeData)
{
    // we suppress the triangle mesh remap table computation to gain some speed, as we will not need it 
// in this snippet
    params.suppressTriangleMeshRemapTable = true;

    // If DISABLE_CLEAN_MESH is set, the mesh is not cleaned during the cooking. The input mesh must be valid. 
    // The following conditions are true for a valid triangle mesh :
    //  1. There are no duplicate vertices(within specified vertexWeldTolerance.See PxCookingParams::meshWeldTolerance)
    //  2. There are no large triangles(within specified PxTolerancesScale.)
    // It is recommended to run a separate validation check in debug/checked builds, see below.

    if (!skipMeshCleanup)
        params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH);
    else
        params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;

    // If DISABLE_ACTIVE_EDGES_PREDOCOMPUTE is set, the cooking does not compute the active (convex) edges, and instead 
    // marks all edges as active. This makes cooking faster but can slow down contact generation. This flag may change 
    // the collision behavior, as all edges of the triangle mesh will now be considered active.
    if (!skipEdgeData)
        params.meshPreprocessParams &= ~static_cast<PxMeshPreprocessingFlags>(PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE);
    else
        params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
}
/*
void PhysX::ProcessCollision(CollisionReport collision)
{
    std::cout << collision.rigidA->getName() << "\n";
}*/