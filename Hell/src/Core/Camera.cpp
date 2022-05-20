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

	m_viewPos = m_inverseViewMatrix[3];// glm::vec3(vP.x, vP.y, vP.z);

	m_projectionViewMatrix = m_projectionMatrix * m_viewMatrix;
}

void Camera::CalculateProjectionMatrix(int screenWidth, int screenHeight)
{
	m_projectionMatrix = glm::perspective(m_fieldOfView, (float)screenWidth / (float)screenHeight, NEAR_PLANE, FAR_PLANE);
	m_inverseProjectionMatrix = glm::inverse(m_projectionMatrix);
}

void Camera::CalculateWeaponSway(float deltaTime, float xOffset, float yOffset, float xMax)
{
	float SWAY_AMOUNT = 0.125f;
	float SMOOTH_AMOUNT = 4.0f;
	float SWAY_MIN_X = -2.25f;
	float SWAY_MAX_X = xMax;//;
	float SWAY_MIN_Y = -1;
	float SWAY_MAX_Y = 0.5f;

	float swayAmount = 1;
	float movementX = -xOffset * SWAY_AMOUNT;
	float movementY = -yOffset * SWAY_AMOUNT;

	

	movementX = std::min(movementX, SWAY_MAX_X);
	movementX = std::max(movementX, SWAY_MIN_X);
	movementY = std::min(movementY, SWAY_MAX_Y);
	movementY = std::max(movementY, SWAY_MIN_Y);

	glm::vec3 finalPosition = glm::vec3(movementX, movementY, 0);

/*	static float rotAmount = 0;

	rotAmount += -Input::m_xoffset * deltaTime * 0.01;

	float limit = 0.025f;
	rotAmount = std::min(rotAmount, limit);
	rotAmount = std::max(rotAmount, -limit);*/

	//m_swayTransform.rotation.y = Util::FInterpTo(rotAmount, m_swayTransform.rotation.y, deltaTime, 1.0f);

	m_swayTransform.position = Util::Vec3InterpTo(m_swayTransform.position, finalPosition, deltaTime, SMOOTH_AMOUNT);
	//m_swayTransform.rotation = Util::Vec3InterpTo(m_swayTransform.rotation, finalPosition, deltaTime, SMOOTH_AMOUNT);

}
