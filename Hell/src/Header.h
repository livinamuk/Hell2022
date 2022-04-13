#pragma once

#define NOMINMAX
#include <Windows.h>

#ifdef _MSC_VER
#undef GetObject
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/hash.hpp"
#include <iostream>
#include <vector>
#include "Renderer/Shader.h"
#include "Core/keycodes.h"
#include <stb_image.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Renderer/Transform.h"
#include <map>
 

//#define SCR_WIDTH 1280//1920//2880 //1920//1280
//#define SCR_HEIGHT 720//1080//1620//1080//720
#define SCR_WIDTH 2880//1920//2880 //1920//1280
#define SCR_HEIGHT 1620//1080//1620//1080//720
#define NEAR_PLANE 0.01f
#define FAR_PLANE 200.0f
#define HELL_PI	(float)3.14159265358979323846
#define EDITOR_GRID_SIZE 0.1f

struct ControllerState
{
	std::vector<bool> buttons_down;
	std::vector<bool> buttons_down_last_frame;
	std::vector<bool> buttons_pressed;
	float left_stick_axis_X = 0;
	float left_stick_axis_Y = 0;
	float right_stick_axis_X = 0;
	float right_stick_axis_Y = 0;
	int axesCount = 0;
};

#define SMALL_NUMBER		(float)9.99999993922529e-9
#define KINDA_SMALL_NUMBER	(float)0.00001

enum class ControllerStickMode {AIMING, MOVEMENT};
enum class InputType {KEYBOARD_AND_MOUSE, CONTROLLER};
enum class CharacterModelAnimationState{ STOPPED, WALKING, STOPPED_CROUCHED, WALKING_CROUCHED };
enum class HUDWeaponAnimationState {IDLE, WALKING, FIRING, RELOADING, EQUIPPING};

/*enum ClipState {
	PLAYING,
	FINISHED,
	PAUSED
};*/

/*
struct AnimationState {
	bool loop = true;
	unsigned int currentIndex = 0;
	float currentTime = 0;
	float speed = 1.0f;
	ClipState clipState;
};*/

struct FileInfo {
	std::string fullpath;
	std::string directory;
	std::string filename;
	std::string filetype;
	std::string materialType;
};

struct ScreenResolution {
	int width = 0;
	int height = 0;

	ScreenResolution() {}

	ScreenResolution(int w, int h) {
		width = w;
		height = h;
	}
};

inline bool FileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

enum FileType { FBX, OBJ, PNG, JPG, UNKNOWN };

struct Vertex2D {
	glm::vec2 Position;
	glm::vec2 TexCoords;
};

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords = glm::vec2(0,0);
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
	unsigned int BlendingIndex[4];
	glm::vec4 BlendingWeight;
	unsigned int MaterialID;

	bool operator==(const Vertex& other) const {
		return Position == other.Position && Normal == other.Normal && TexCoords == other.TexCoords;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.TexCoords) << 1);
		}
	};
}

/*struct Line {
	glm::vec3 start_pos;
	glm::vec3 end_pos;
	glm::vec3 start_color;
	glm::vec3 end_color;
};*/

enum class PhysicsObjectType { UNDEFINED, WALL, EDITOR_WINDOW, DOOR, FLOOR, STAIRS, PLAYER_RAGDOLL, RAGDOLL, CEILING, SHELL_PROJECTILE, MISC_MESH, GLASS };

struct EntityData {

	EntityData(PhysicsObjectType type, void* parent) {
		this->type = type;
		this->parent = parent;
	}

	PhysicsObjectType type;
	void* parent;
};

/*enum class CollisionGroups {
	NONE = 0,
	TERRAIN = 1 << 0,
	STATIC_ENTITY = 1 << 1,
	PLAYER_1 = 1 << 2,
	PLAYER_2 = 2 << 3,
};*/

enum ActiveGroup
{
	//NONE = 0,
	GROUP_RAYCAST = (1 << 0),
	//GROUP2 = (1 << 1),
	//GROUP3 = (1 << 2),
	//GROUP4 = (1 << 3),
};

#define WORLD_GROUND_SIZE	150.0f
#define WORLD_GROUND_HEIGHT	1.0f

#define DEBUG_COLOR_DOOR btVector3(0.4f, 0.5f, 0.5f)
#define DEBUG_COLOR_WALL btVector3(0.6f, 0.5f, 0.5f)
#define DEBUG_COLOR_RAMP btVector3(0.9f, 0.8f, 0.8f)
#define DEBUG_COLOR_YELLOW btVector3(1.0f, 1.0f, 0.0f)
#define DEBUG_COLOR_GROUND btVector3(0.2f, 0.2f, 0.2f)
#define DEBUG_COLOR_STATIC_ENTITY btVector3(1, 1, 0)

#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }
#define ToRadian(x) (float)(((x) * HELL_PI / 180.0f))
#define ToDegree(x) (float)(((x) * 180.0f / HELL_PI)) 

#define POSITION_LOCATION    0
#define NORMAL_LOCATION		 1
#define TEX_COORD_LOCATION   2
#define TANGENT_LOCATION     3
#define BITANGENT_LOCATION   4
#define BONE_ID_LOCATION     5
#define BONE_WEIGHT_LOCATION 6

enum VB_TYPES {
	INDEX_BUFFER,
	POS_VB,
	NORMAL_VB,
	TEXCOORD_VB,
	TANGENT_VB,
	BITANGENT_VB,
	BONE_VB,
	NUM_VBs
};  