#pragma once
#include "Core/Input.h"
#include "Renderer/Transform.h"

class Camera
{
public:	// Methods
	Camera();
	void CalculateMatrices(glm::mat4 animatedCameraMatrix = glm::mat4(1));
	void CalculateProjectionMatrix(int screenWidth, int screenHeight);
	//void CalculateWeaponSwayTransform(float deltatime);
	//void Update(float deltaTime);

private: // Methods

public: // Fields

	Transform m_transform;

	glm::vec3 m_Front = glm::vec3(0, 1, 0);
	glm::vec3 m_Up;
	glm::vec3 m_Right;
	glm::vec3 m_WorldUp;

	glm::mat4 m_projectionViewMatrix;
	glm::mat4 m_projectionMatrix;
	glm::mat4 m_viewMatrix;
	glm::mat4 m_inverseViewMatrix;
	glm::mat4 m_inversePprojectionMatrix;
		
	glm::vec3 m_viewPos = glm::vec3(0);
};