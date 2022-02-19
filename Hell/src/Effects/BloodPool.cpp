#include "BloodPool.h"
#include "Helpers/AssetManager.h"

BloodPool::BloodPool()
{
    // Fuck
}

BloodPool::BloodPool(Transform transform)
{
	m_transform = transform;
    m_transform.scale = glm::vec3(0.25);
}

void BloodPool::Update(float deltaTime)
{
    if (m_transform.scale.x >= 0.75)
        return;

    float ahimSpeed = 0.5;
    m_transform.scale += glm::vec3(deltaTime * ahimSpeed);
}

void BloodPool::Draw(Shader* shader)
{
    Material* material = AssetManager::GetMaterialPtr("BloodPool");
    material->Bind();
    Util::DrawUpFacingPlane(shader, m_transform.to_mat4());
}
