#include "Door.h"
#include "Renderer/Renderer.h"
#include "Helpers/AssetManager.h"

#include "Core/GameData.h"

//physx::PxShape* Door::s_doorShape;
//physx::PxShape* Door::s_frameShape;

#define SHAPE_WIDTH (DOOR_WIDTH * 0.5f)
#define SHAPE_HEIGHT (DOOR_HEIGHT * 0.5f)
#define SHAPE_DEPTH (DOOR_DEPTH * 0.5f)

#define DOOR_OPEN_SPEED 3.0f

void Door::CreateShapes()
{
    physx::PxPhysics* physics = PhysX::GetPhysics();
    physx::PxMaterial* material = PhysX::GetDefaultMaterial();
    physx::PxScene* scene = PhysX::GetScene();

    float graceFactor = 0.05;
}

Door::Door()
{
    m_OffsetTransform.position.z = -0.05;
    m_physicalDoorOffsetTransform.position.y = 1.0025;
    m_physicalDoorOffsetTransform.position.z = 0.0434;

   // CreateCollisionObject();
}

Door::Door(glm::vec3 position)
{
    m_transform.position = position;
    m_physicalDoorOffsetTransform.position.y = 1.0025;
    m_physicalDoorOffsetTransform.position.z = 0.0434;

}

void Door::Draw(Shader* shader)
{
    AssetManager::GetMaterialPtr("Door")->Bind();
    //AssetManager::m_models["Door"].Draw(shader, m_transform.to_mat4() * m_OffsetTransform.to_mat4() * m_physicalDoorOffsetTransform.to_mat4());

    //glm::mat4 modelMatrix = Util::PxMat44ToGlmMat4(m_rigid->getGlobalPose());

    AssetManager::m_models["Door"].Draw(shader, GetModelMatrix());


    AssetManager::GetMaterialPtr("DoorFrame")->Bind();
    AssetManager::m_models["DoorFrame"].Draw(shader, m_transform.to_mat4() * m_OffsetTransform.to_mat4() );

    // Floor
    AssetManager::GetMaterialPtr("FloorBoards")->Bind();
    Transform floorBoardsTransform;
    floorBoardsTransform.scale = glm::vec3(DOOR_WIDTH, 1, 0.1);
    shader->setInt("u_TEXCOORD_FLAG", 1);
    Util::DrawUpFacingPlane(shader, m_transform.to_mat4() * m_OffsetTransform.to_mat4() * floorBoardsTransform.to_mat4());
    shader->setInt("u_TEXCOORD_FLAG", 0);
}

glm::vec3 TranslationFromMatrix(glm::mat4 m)
{
    return glm::vec3(m[3]);
}

void Door::DrawForEditor(Shader* shader, glm::vec3 planeColor)
{
    shader->setVec3("u_color", planeColor);
    Transform scaleTransform;
    scaleTransform.scale = glm::vec3(DOOR_WIDTH, 1, 0.1f);
    Util::DrawUpFacingPlane(shader, m_transform.to_mat4() * m_OffsetTransform.to_mat4() * scaleTransform.to_mat4());

    // v1      v3
    // 
    // v2      v4
    glm::vec3 v1 = v1 = GetVert1();
    glm::vec3 v2 = v2 = GetVert2();
    glm::vec3 v3 = v3 = GetVert3();
    glm::vec3 v4 = v4 = GetVert4();
 
    Renderer::DrawLine(shader, v1, v2, glm::vec3(0, 1, 1));
    Renderer::DrawLine(shader, v2, v3, glm::vec3(0, 1, 1));
    Renderer::DrawLine(shader, v3, v4, glm::vec3(0, 1, 1));
    Renderer::DrawLine(shader, v4, v1, glm::vec3(0, 1, 1));

    // Left side
  /*  Transform t(GetVert1(glm::vec3(0, 0, -0.05f)));
    Transform t2(GetVert2(glm::vec3(0, 0, 0.05f)));

    // Right side
    Transform t3(GetVert3(glm::vec3(0, 0, 0.05f)));
    Transform t4(GetVert4(glm::vec3(0, 0, -0.05f)));

    Renderer::DrawPoint(shader, t.to_mat4(), YELLOW);
    Renderer::DrawPoint(shader, t2.to_mat4(), RED);
    Renderer::DrawPoint(shader, t3.to_mat4(), BLUE);
    Renderer::DrawPoint(shader, t4.to_mat4(), GREEN);*/
}

glm::vec3 Door::GetVert1(glm::vec3 localOffset)
{
    glm::vec3 v1 = glm::vec3(
        -DOOR_WIDTH / 2 + localOffset.x,
        localOffset.y,
        -0.05 + localOffset.z);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v1).to_mat4() * m_OffsetTransform.to_mat4());
}

glm::vec3 Door::GetVert2(glm::vec3 localOffset)
{
    glm::vec3 v2 = glm::vec3(
        -DOOR_WIDTH / 2 + localOffset.x,
        localOffset.y,
        0.05 + localOffset.z);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v2).to_mat4() * m_OffsetTransform.to_mat4());
}

glm::vec3 Door::GetVert3(glm::vec3 localOffset)
{
    glm::vec3 v3 = glm::vec3(
        DOOR_WIDTH / 2 + localOffset.x,
        localOffset.y,
        0.05 + localOffset.z);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v3).to_mat4() * m_OffsetTransform.to_mat4());
}
glm::vec3 Door::GetVert4(glm::vec3 localOffset)
{
    glm::vec3 v4 = glm::vec3(
        DOOR_WIDTH / 2 + localOffset.x,
        localOffset.y,
        -0.05 + localOffset.z);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v4).to_mat4() * m_OffsetTransform.to_mat4());
}

glm::vec3 Door::GetParentVert1()
{
    Room* parentRoom = &GameData::s_rooms[m_parentRoomIndex];
    return parentRoom->m_vertices[m_parentIndexVertexA];
}

glm::vec3 Door::GetParentVert2()
{
    Room* parentRoom = &GameData::s_rooms[m_parentRoomIndex];
    return parentRoom->m_vertices[m_parentIndexVertexB];
}

glm::vec3* Door::GetPointerToParentVert1()
{
    Room* parentRoom = &GameData::s_rooms[m_parentRoomIndex];
    return &parentRoom->m_vertices[m_parentIndexVertexA];
}

glm::vec3* Door::GetPointerToParentVert2()
{
    Room* parentRoom = &GameData::s_rooms[m_parentRoomIndex];
    return &parentRoom->m_vertices[m_parentIndexVertexB];
}

void Door::Interact()
{
	if (m_state == State::CLOSING) {
		m_state = State::OPENING;
		Audio::PlayAudio("Door_Open.wav", 0.5f);
        m_closed = false;
        return;
	}

	if (m_state == State::OPENING) {
		m_state = State::CLOSING;
		m_closed = false;
		Audio::PlayAudio("Door_Open.wav", 0.5f);
		return;
	}
    /*
    if (m_state == State::CLOSED) {
        m_state = State::OPENING;
        Audio::PlayAudio("Door_Open.wav");
    }    
    
    if (m_state == State::OPEN) {
        m_state = State::CLOSING;
        Audio::PlayAudio("Door_Open.wav");
    }*/
}

void Door::Update(float deltaTime)
{
	// revalidate the physics pointer
	if(m_rigid)
        m_rigid->userData = new EntityData(PhysicsObjectType::DOOR, this);

	float swingOverShoot = 0.75f;

    float amount = deltaTime + Util::RandomFloat(-deltaTime*0.5, deltaTime*0.5);

	if (m_state == State::OPENING)
		m_swing = Util::FInterpTo(m_swing, m_swingMaxAngle + 0.5, amount, 3.5);
	if (m_state == State::CLOSING)
		m_swing = Util::FInterpTo(m_swing, 0 - swingOverShoot, amount, 2.5);

    // Actually cap to  the range
	m_swing = std::min(m_swing, m_swingMaxAngle);
	m_swing = std::max(m_swing, 0.0f);

    m_rigid->setGlobalPose(PxTransform(Util::GlmMat4ToPxMat44(GetModelMatrix())));


    // Play latch sound on door close
    if (!m_closed && m_state == State::CLOSING && m_swing < 0.2) {
        m_closed = true;
		Audio::PlayAudio("Door_Latch.wav", 0.9f);
    }

    return;

    if (m_state == State::OPENING)
    {
        glm::mat4 mat = Util::PxMat44ToGlmMat4(m_rigid->getGlobalPose());
        glm::vec3 basisZ = mat[2] * DOOR_OPEN_SPEED;
        PxVec3 force = PxVec3(basisZ.x, basisZ.y, basisZ.z);
        m_rigid->addForce(force);

        if (m_joint->getSwingYAngle() < -2)
        {
            m_state = State::OPEN;
            m_rigid->setAngularVelocity(PxVec3(0, 0, 0));
            m_rigid->putToSleep();
        }
    }    

    if (m_state == State::CLOSING)
    {
        glm::mat4 mat = Util::PxMat44ToGlmMat4(m_rigid->getGlobalPose());
        glm::vec3 basisZ = mat[2] * -DOOR_OPEN_SPEED;
        PxVec3 force = PxVec3(basisZ.x, basisZ.y, basisZ.z);
        m_rigid->addForce(force);

        if (m_joint->getSwingYAngle() >= 0)
        {
            m_state = State::CLOSED;
            m_rigid->setAngularVelocity(PxVec3(0, 0, 0));
            m_rigid->putToSleep();
            m_rigid->setGlobalPose(m_restPose);
        }
    }

   
}

void Door::CreateCollisionObject()
{   
    // physx
    PxPhysics* physX = PhysX::GetPhysics();
    PxMaterial* material = PhysX::GetDefaultMaterial();
    PxScene* scene = PhysX::GetScene();

	m_shape = PhysX::GetPhysics()->createShape(physx::PxBoxGeometry(SHAPE_WIDTH, SHAPE_HEIGHT * 0.95, SHAPE_DEPTH), *material);

	PxFilterData filterData;
	filterData.word1 = PhysX::CollisionGroup::MISC_OBSTACLE;
	m_shape->setQueryFilterData(filterData);

    glm::mat4 spawnMat = m_transform.to_mat4() * m_OffsetTransform.to_mat4() * m_physicalDoorOffsetTransform.to_mat4();
    PxMat44 spawnMatrix = Util::GlmMat4ToPxMat44(spawnMat);
    m_rigid = physX->createRigidDynamic(PxTransform(spawnMatrix));
    m_rigid->attachShape(*m_shape);
    m_rigid->userData = new EntityData(PhysicsObjectType::DOOR, this);

    float mass = 0.5f;
    PxRigidBodyExt::setMassAndUpdateInertia(*m_rigid, mass);
    //s_doorShape->release();

    PxFilterData data;
    //data.word0 = CollisionGroups::NONE;
    //data.word1 = PhysX::CollisionGroup::DOOR;
   // m_shape->setQueryFilterData(data);

    PhysX::EnableRayCastingForShape(m_shape);

    PxShape* frameShape = physX->createShape(PxBoxGeometry(0.025, DOOR_HEIGHT * 0.005, 0.05), *material);
    m_frameRigid = physX->createRigidStatic(PxTransform(spawnMatrix));
    m_frameRigid->attachShape(*frameShape);
    m_frameRigid->userData = new EntityData(PhysicsObjectType::DOOR, this);

    PxTransform parentFrame = PxTransform(PxVec3(-SHAPE_WIDTH, 0, 0.00));
    PxTransform childFrame = PxTransform(PxVec3(-SHAPE_WIDTH, 0.0f, SHAPE_DEPTH));
    m_joint = PxD6JointCreate(*physX, m_frameRigid, parentFrame, m_rigid, childFrame);
    m_joint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, false);
    m_joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, true);

    scene->addActor(*m_rigid);
    scene->addActor(*m_frameRigid);

    m_rigid->setName("DOOR");

    m_joint->setMotion(PxD6Axis::eTWIST, PxD6Motion::eLOCKED);
    m_joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
    m_joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);

    m_restPose = m_rigid->getGlobalPose();
    m_rigid->setAngularVelocity(PxVec3(0, 0, 0));
    m_rigid->putToSleep();
}

void Door::RemoveCollisionObject()
{      
    m_state = State::CLOSED;
    m_rigid->release();
}

glm::mat4 Door::GetModelMatrix()
{
	Transform rotTransform;;
	rotTransform.position.y += 1;
	rotTransform.rotation.y = -m_swing;

	Transform transIn(glm::vec3(DOOR_WIDTH / 2, 0, 0));
	Transform transOut(glm::vec3(-DOOR_WIDTH / 2, 0, 0));

	return m_transform.to_mat4() * transOut.to_mat4() * rotTransform.to_mat4() * transIn.to_mat4();
}
