#pragma once
#include "Header.h"

// Fuck

class BloodPool
{
public: // methods
	BloodPool();
	BloodPool(Transform transform);
	void Update(float deltaTime);
	void Draw(Shader* shader);

public: // fields
	Transform m_transform;
};