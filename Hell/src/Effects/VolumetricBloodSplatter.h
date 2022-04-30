#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/Transform.h"
#include "header.h"
#include "Renderer/Model.h"
#include "Helpers/AssetManager.h"

class VolumetricBloodSplatter
{
public: // fields
	float m_CurrentTime = 0.0f;
	Transform m_transform;
	Model* m_model;
	glm::vec3 m_front;

	int m_type = 9;

public: // methods
	VolumetricBloodSplatter(glm::vec3 position, glm::vec3 rotation, glm::vec3 front);
	void Update(float deltaTime);
	void Draw(Shader* shader);
	glm::mat4 GetModelMatrix();

public: // static functions
	//static void Init();

public: // static Variables
	static GLuint s_buffer_mode_matrices;
	static GLuint s_vao;

	static unsigned int s_counter;
	//static std::vector<VolumetricBloodSplatter> s_volumetricBloodSplatters;
};
