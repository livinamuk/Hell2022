#pragma once
#include "Header.h"

class Door
{
public:
	Transform m_transform;
	Transform m_OffsetTransform;
	//glm::vec3* m_parentVertexA;
	//glm::vec3* m_parentVertexB;

	unsigned int m_parentIndexVertexA;
	unsigned int m_parentIndexVertexB;
	unsigned int m_parentRoomIndex;
	
	//float m_distanceFromVertexA;
	//void* m_parentRoom;

	Door();
	Door(glm::vec3 position);
	void Draw(Shader* shader);
	void DrawForEditor(Shader* shader, glm::vec3 planeColor);

	glm::vec3 GetVert1();
	glm::vec3 GetVert2();
	glm::vec3 GetVert3();
	glm::vec3 GetVert4();

	glm::vec3 GetParentVert1();
	glm::vec3 GetParentVert2();
	glm::vec3* GetPointerToParentVert1();
	glm::vec3* GetPointerToParentVert2();
};