#pragma once

#include "Renderer/Transform.h"
#include "Header.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/Shader.h"

class Decal
{
public: // fields
	Transform transform;
	glm::vec3 normal;

public: // methods
	Decal(glm::vec3 position, glm::vec3 normal);
	void Draw(Shader* shader, bool blackOnly);

	static std::vector<Decal> s_decals;
};
