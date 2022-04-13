#include "Door.h"
#include "Renderer/Renderer.h"
#include "Helpers/AssetManager.h"

#include "Core/GameData.h"

#define DOOR_WIDTH 0.8f
#define DOOR_DEPTH 0.105f

Door::Door()
{
    m_OffsetTransform.position.z = -0.05;
}

Door::Door(glm::vec3 position)
{
    m_transform.position = position; 
    m_OffsetTransform.position.z = -0.05;
}

void Door::Draw(Shader* shader)
{
    Transform localTranslation;
    localTranslation.position.y = 1;
    AssetManager::GetMaterialPtr("Door")->Bind();
    AssetManager::m_models["Door"].Draw(shader, m_transform.to_mat4() * m_OffsetTransform.to_mat4() * localTranslation.to_mat4());

    AssetManager::GetMaterialPtr("DoorFrame")->Bind();
    AssetManager::m_models["DoorFrame"].Draw(shader, m_transform.to_mat4() * m_OffsetTransform.to_mat4() );
}

glm::vec3 TranslationFromMatrix(glm::mat4 m)
{
    return glm::vec3(m[3]);
}

void Door::DrawForEditor(Shader* shader, glm::vec3 planeColor)
{
    shader->setVec3("u_color", planeColor);
    Transform scaleTransform;
    scaleTransform.scale = glm::vec3(DOOR_WIDTH, 1, DOOR_DEPTH);
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
}

glm::vec3 Door::GetVert1()
{
    glm::vec3 v1 = glm::vec3(-DOOR_WIDTH / 2, 0, -DOOR_DEPTH / 2);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v1).to_mat4() * m_OffsetTransform.to_mat4());
}

glm::vec3 Door::GetVert2()
{
    glm::vec3 v2 = glm::vec3(-DOOR_WIDTH / 2, 0, DOOR_DEPTH / 2);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v2).to_mat4() * m_OffsetTransform.to_mat4());
}

glm::vec3 Door::GetVert3()
{
    glm::vec3 v3 = glm::vec3(DOOR_WIDTH / 2, 0, DOOR_DEPTH / 2);
    return TranslationFromMatrix(m_transform.to_mat4() * Transform(v3).to_mat4() * m_OffsetTransform.to_mat4());
}
glm::vec3 Door::GetVert4()
{
    glm::vec3 v4 = glm::vec3(DOOR_WIDTH / 2, 0, -DOOR_DEPTH / 2);
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
