#pragma once
//#include "hellpch.h"
#include "Camera.h"
#include "Header.h"
#include "Helpers/Util.h"
#include "Core/CoreGL.h"
//#include "Core/CoreImGui.h"
//#include "Config.h"

Camera::Camera()
{
	m_transform.position = glm::vec3(0, 0, 0);
	m_transform.rotation = glm::vec3(-HELL_PI, -3.14, 0);
	//m_transform.rotation = glm::vec3(0 - 0.15f, 0, 0);
	m_transform.rotation = glm::vec3(0 - 0.15f, 0, 0);
	m_transform.scale = glm::vec3(1);
}

void Camera::CalculateMatrices(glm::mat4 animatedCameraMatrix)
{
	m_viewMatrix = animatedCameraMatrix * glm::inverse(m_transform.to_mat4());
	m_inverseViewMatrix = glm::inverse(m_viewMatrix);

	m_Right = glm::vec3(m_inverseViewMatrix[0]);//*glm::vec3(-1, -1, -1);
	m_Up = glm::vec3(m_inverseViewMatrix[1]);// *glm::vec3(-1, -1, -1);
	m_Front = glm::vec3(m_inverseViewMatrix[2]) * glm::vec3(-1, -1, -1);

	glm::vec4 vP = (m_inverseViewMatrix * glm::vec4(0, 0, 0, 1));
	m_viewPos = m_viewMatrix[3];// glm::vec3(vP.x, vP.y, vP.z);

	m_projectionViewMatrix = m_projectionMatrix * m_viewMatrix;
}

void Camera::CalculateProjectionMatrix(int screenWidth, int screenHeight)
{
	m_projectionMatrix = glm::perspective(1.0f, (float)screenWidth / (float)screenHeight, NEAR_PLANE, FAR_PLANE);
	m_inversePprojectionMatrix = glm::inverse(m_projectionMatrix);
}