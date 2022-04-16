#pragma once
#include "header.h"
#include "Core/GameData.h"

struct EditorPoint {
	float x;
	float z;

	EditorPoint() {};

	EditorPoint(float x, float z) {
		this->x = x;
		this->z = z;
	}
};

struct Line {
	EditorPoint start;
	EditorPoint end;
};


enum class Action {
	IDLE,
	CREATING_ROOM,
	PLACING_DOOR
};

enum class DragState {
	IDLE,
	DRAGGING_VERTEX,
	DRAGGING_DOOR
};

enum class SelectedState {
	IDLE,
	VERTEX_SELECTED,
	DOOR_SELECTED
};

static struct SelectedVertex {
	Room* room;
	int index;
} s_selectedVertex;

static struct HoveredDoor {
	Door* door;

	bool WasFound() {
		return (door != nullptr);
	}
} s_hoveredDoor;

static struct HoveredLine {
	bool found = false;
	Room* room = nullptr;
	glm::vec3* v1;
	glm::vec3* v2;
	unsigned int vertIndex1 = -1;
	unsigned int vertIndex2 = -1;

	glm::vec3 GetVert1() {
		return room->m_vertices[vertIndex1];
	}
	glm::vec3 GetVert2() {
		return room->m_vertices[vertIndex2];
	}

} s_hoveredLine;

static struct HoveredVertex {
	bool found = false;
	Room* room = nullptr;
	glm::vec3* position;
} s_hoveredVertex;


class Editor
{
public: // methods
	static void Update(float screenWidth, float screenHeight);
	static void Render(float screenwidth, float screenheight);
	static void SubmitBlitterText();
	static void ToggleEditor();
	static bool IsOpen();
	//static Room s_newRoom;

private:
	static bool VertexEquality(glm::vec3 a, glm::vec3 b);
	static glm::vec3* VertexExistsAtThisLocation(glm::vec3 query);

	static void CheckForHoveredDoor();
	static void CheckForHoveredLine(); 

	static float Minimum_Distance(glm::vec2 v, glm::vec2 w, glm::vec2 p);
	static glm::vec3 ClosestPointOnLine(glm::vec3 point, glm::vec3 start, glm::vec3 end);
	static void ResetSelectedAndDragStates();
	static void DragSelectedVertex();
	static void DeleteSelectedVertex();
	static void DragSelectedDoor();
	static void ResetEditorState();

	static glm::vec3 GetMouseGridPosVec3();
	static glm::vec3 GetMousePosVec3();
	static glm::vec3 GetClosestPointFromMouseToHoveredLine();
	static void PlaceDoorAtMousePos();

public: //
	static void ReCalculateAllDoorPositions();
	static void RebuildAllMeshData();

private: // fields
	static bool s_isOpen;
	static float s_zoom;
	static float s_cameraX;
	static float s_cameraZ;
	static float s_scrollSpeed;
	static float s_mouseX;
	static float s_mouseZ;
	static float s_gridX;
	static float s_gridZ;
	static std::vector<glm::vec3> s_newRoomVertices;
};