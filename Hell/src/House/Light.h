#pragma once
#include "Header.h"
#include "Renderer/EnvMap.h"
#include "Renderer/ShadowMap.h"

#define DEFAULT_LIGHT_COLOR glm::vec3(1, 0.8, 0.529)

class Light
{
public:
	glm::vec3 m_position;
	Transform m_modelTransform = Transform(glm::vec3(0, 0.2, 0));
	glm::vec3 m_color = glm::vec3(1, 0.8, 0.529);
	float m_radius;
	float m_strength;
	float m_magic;
	int m_modelType = 0;
	std::vector<glm::mat4> m_projectionTransforms;

	ShadowMap m_shadowMap;
	ShadowMap m_indirectShadowMap;
	EnvMap m_envMap;

	Light();
	Light(glm::vec3 position, glm::vec3 color, float m_radius, float m_strength, float m_magic, int modelType);
	void Draw(Shader* shader);
	void CalculateProjectionTransforms();

	//ShadowMap m_shadowMapStorage;
	//ShadowMap m_indirectShadowMapStorage;
	//LightProbe m_lightProbeStorage;
};