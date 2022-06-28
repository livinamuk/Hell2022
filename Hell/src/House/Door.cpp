#include "Door.h"
#include "Renderer/Renderer.h"
#include "Helpers/AssetManager.h"

#include "Core/GameData.h"

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
    // try commenting this whole constructor out one time. perhaps its never used
    m_transform.position = position;
    m_physicalDoorOffsetTransform.position.y = 1.0025;
    m_physicalDoorOffsetTransform.position.z = 0.0434;

}

void Door::Draw(Shader* shader)
{
    Transform trueOffet = m_OffsetTransform;

    if (m_mirror) {
        trueOffet.rotation.y += HELL_PI;
        //trueOffet.position.z *= -1;
    }

    // Is it a door?
    if (m_type == Type::DOOR) {
        AssetManager::GetMaterialPtr("Door")->Bind();
        AssetManager::m_models["Door"].Draw(shader, GetModelMatrix());
        AssetManager::GetMaterialPtr("DoorFrame")->Bind();
        AssetManager::m_models["DoorFrame"].Draw(shader, m_transform.to_mat4() * trueOffet.to_mat4());

        // door has a floor too...
        AssetManager::GetMaterialPtr("FloorBoards")->Bind();
        Transform floorBoardsTransform;
        floorBoardsTransform.scale = glm::vec3(DOOR_WIDTH, 1, 0.1);
        shader->setInt("u_TEXCOORD_FLAG", 1);
        Util::DrawUpFacingPlane(shader, m_transform.to_mat4() * trueOffet.to_mat4() * floorBoardsTransform.to_mat4());
        shader->setInt("u_TEXCOORD_FLAG", 0);
    }
    // Otherwise it's a window
    else {

        AssetManager::GetMaterialPtr("WindowExterior")->Bind();
        AssetManager::GetMaterialPtr("Window")->BindToSecondSlot();
        glm::mat4 modelMatrix = m_transform.to_mat4() * trueOffet.to_mat4();

        if (m_type == Type::WINDOW_SINGLE) {
            Model* model = &AssetManager::m_models["Window"];
            model->DrawMesh(shader, 2, modelMatrix);
            model->DrawMesh(shader, 3, modelMatrix);
            model->DrawMesh(shader, 4, modelMatrix);
            model->DrawMesh(shader, 5, modelMatrix);
        }
        else if (m_type == Type::WINDOW_DOUBLE) {
            Model* model = &AssetManager::m_models["WindowDouble"];
            model->DrawMesh(shader, 0, modelMatrix);
            model->DrawMesh(shader, 1, modelMatrix);
            model->DrawMesh(shader, 3, modelMatrix);
            model->DrawMesh(shader, 5, modelMatrix);
            model->DrawMesh(shader, 6, modelMatrix);
            model->DrawMesh(shader, 8, modelMatrix);
            model->DrawMesh(shader, 9, modelMatrix);
        }
    }
}

void Door::NextType()
{
    if (m_type == Type::DOOR) {
        m_type = Type::WINDOW_SINGLE;
        RemoveCollisionObject(); 
        CreateCollisionObject();
        return;
    }
    else if (m_type == Type::WINDOW_SINGLE) {
        m_type = Type::WINDOW_DOUBLE;
        RemoveCollisionObject();
        CreateCollisionObject();
        return;
    }
    else if (m_type == Type::WINDOW_DOUBLE) {
        m_type = Type::DOOR;
        RemoveCollisionObject();
        CreateCollisionObject();
        return;
    }
}

void Door::Rotate180()
{
    m_mirror = !m_mirror;

  /*  m_transform.rotation.y += HELL_PI;
    m_OffsetTransform.position.z *= -1;*/

    RemoveCollisionObject();
    CreateCollisionObject();
}

float Door::GetWidth()
{
    if (m_type == Type::DOOR)
        return DOOR_WIDTH;
    else if (m_type == Type::WINDOW_SINGLE)
        return WINDOW_WIDTH_SINGLE;
    else
        return WINDOW_WIDTH_DOUBLE;
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
 
    glm::vec3 color = glm::vec3(0, 1, 1);

    if (m_type != Type::DOOR)
        color = glm::vec3(1, 1, 0);

    Renderer::DrawLine(shader, v1, v2, color);
    Renderer::DrawLine(shader, v2, v3, color);
    Renderer::DrawLine(shader, v3, v4, color);
    Renderer::DrawLine(shader, v4, v1, color);

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
        -GetWidth() / 2 + localOffset.x,
        localOffset.y,
        -0.05 + localOffset.z);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v1).to_mat4() * m_OffsetTransform.to_mat4());
}

glm::vec3 Door::GetVert2(glm::vec3 localOffset)
{
    glm::vec3 v2 = glm::vec3(
        -GetWidth() / 2 + localOffset.x,
        localOffset.y,
        0.05 + localOffset.z);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v2).to_mat4() * m_OffsetTransform.to_mat4());
}

glm::vec3 Door::GetVert3(glm::vec3 localOffset)
{
    glm::vec3 v3 = glm::vec3(
        GetWidth() / 2 + localOffset.x,
        localOffset.y,
        0.05 + localOffset.z);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v3).to_mat4() * m_OffsetTransform.to_mat4());
}
glm::vec3 Door::GetVert4(glm::vec3 localOffset)
{
    glm::vec3 v4 = glm::vec3(
        GetWidth() / 2 + localOffset.x,
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
    if (m_type != Type::DOOR)
        return;

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
}

void Door::Update(float deltaTime)
{
    if (m_type != Type::DOOR) {
        m_swing = 0;
        return;
    }


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
}

void Door::CreateCollisionObject()
{
    // Create rigid
    if (m_type == Type::DOOR)
    {
        glm::mat4 spawnMat = m_transform.to_mat4() * m_OffsetTransform.to_mat4() * m_physicalDoorOffsetTransform.to_mat4();
        PxMat44 spawnMatrix = Util::GlmMat4ToPxMat44(spawnMat);
        PxBoxGeometry geom = PxBoxGeometry(SHAPE_WIDTH, SHAPE_HEIGHT * 0.95, SHAPE_DEPTH);
        PxTransform pose = PxTransform(spawnMatrix);
        PxMaterial* material = PhysX::GetDefaultMaterial();
        m_rigid = PxCreateStatic(*PhysX::GetPhysics(), pose, geom, *material);
        m_rigid->userData = new EntityData(PhysicsObjectType::DOOR, this);
        m_rigid->setName("DOOR");
        PhysX::GetScene()->addActor(*m_rigid);

        // set the filter data
        PxShape* shape;
        m_rigid->getShapes(&shape, 1);
        PxFilterData filterData;
        filterData.word1 = PhysX::CollisionGroup::MISC_OBSTACLE;
        shape->setQueryFilterData(filterData);
        PhysX::EnableRayCastingForShape(shape);

        // store rest pose, for resetting the door back to fully closed.
        m_restPose = m_rigid->getGlobalPose();
    }
}

void Door::RemoveCollisionObject()
{      
    if (m_rigid)
        m_rigid->release();

    m_rigid = nullptr;
}

glm::mat4 Door::GetModelMatrix()
{
	Transform rotTransform;
	rotTransform.position.y += 1;
	rotTransform.rotation.y = -m_swing;

    Transform mirrorTransform;
    if (m_mirror) {
        mirrorTransform.rotation.y += HELL_PI;
        mirrorTransform.position.z -= 0.1f;
    }

	Transform transIn(glm::vec3(DOOR_WIDTH / 2, 0, 0));
	Transform transOut(glm::vec3(-DOOR_WIDTH / 2, 0, 0));

	return m_transform.to_mat4() * mirrorTransform.to_mat4() * transOut.to_mat4() * rotTransform.to_mat4() * transIn.to_mat4();
}
