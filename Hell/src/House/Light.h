#pragma once
#include "Header.h"
#include "Renderer/EnvMap.h"
#include "Renderer/ShadowMap.h"

#define DEFAULT_LIGHT_COLOR glm::vec3(1, 0.8, 0.529)
#define COLOR_RED glm::vec3(1, 0, 0)

class Light
{
public:
	glm::vec3 m_position = glm::vec3(0,0,0);
	Transform m_modelTransform = Transform(glm::vec3(0, 0.2, 0));
	glm::vec3 m_color = glm::vec3(1, 0.8, 0.529);
	float m_radius = 0;
	float m_strength = 0;
	float m_magic = 0;
	int m_modelType = 0;
	std::vector<glm::mat4> m_projectionTransforms;
	bool m_needsUpadte = true;

	ShadowMap m_shadowMap;
	ShadowMap m_indirectShadowMap;
	EnvMap m_envMap;

	Light();
	Light(glm::vec3 position, glm::vec3 color, float m_radius, float m_strength, float m_magic, int modelType);
	void Draw(Shader* shader);
	void CalculateProjectionTransforms(); 
	void CleanUp();

	//ShadowMap m_shadowMapStorage;
	//ShadowMap m_indirectShadowMapStorage;
	//LightProbe m_lightProbeStorage;
};