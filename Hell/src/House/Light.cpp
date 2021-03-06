#include "Light.h"
#include "Helpers/AssetManager.h"


Light::Light()
{
}

Light::Light(glm::vec3 position, glm::vec3 color, float radius, float strength, float magic, int modelType)
{
	m_position = position;
	m_color = color;
	m_radius = radius;
	m_strength = strength;
	m_magic = magic;
	m_modelType = modelType;

//	m_color = glm::vec3(1, 0.7799999713897705, 0.5289999842643738);
//	m_radius = 6;
	//m_strength = 10;
	//m_magic = 4;
	//m_position.y = 2;// = vec3(0, 2, 0);

	m_modelTransform.position = glm::vec3(0, 0.22f, 0);
	m_modelTransform.scale = glm::vec3(0.35f);

	m_envMap.Init();
	m_shadowMap.Init();
	m_indirectShadowMap.Init();
	CalculateProjectionTransforms();
}

void Light::CleanUp()
{
	m_envMap.CleanUp();
	m_shadowMap.CleanUp();
	m_indirectShadowMap.CleanUp();
}

void Light::Draw(Shader* shader)
{
	AssetManager::GetMaterialPtr("Light")->Bind();
	AssetManager::m_models["Light_04"].Draw(shader, Transform(m_position).to_mat4() * m_modelTransform.to_mat4());
}

void Light::CalculateProjectionTransforms()
{
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_MAP_SIZE / (float)SHADOW_MAP_SIZE, SHADOW_NEAR_PLANE, SHADOW_FAR_PLANE);
	m_projectionTransforms.clear();
	m_projectionTransforms.push_back(shadowProj * glm::lookAt(m_position, m_position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	m_projectionTransforms.push_back(shadowProj * glm::lookAt(m_position, m_position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	m_projectionTransforms.push_back(shadowProj * glm::lookAt(m_position, m_position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	m_projectionTransforms.push_back(shadowProj * glm::lookAt(m_position, m_position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	m_projectionTransforms.push_back(shadowProj * glm::lookAt(m_position, m_position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	m_projectionTransforms.push_back(shadowProj * glm::lookAt(m_position, m_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
}