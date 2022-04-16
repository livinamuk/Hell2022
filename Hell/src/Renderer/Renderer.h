#pragma once
#include "Renderer/Decal.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/Model.h"
#include "Core/Camera.h"
#include "Core/Player.h"
#include "Core/CoreGL.h"
#include "Core/Entity.h"
#include "Core/Scene.h"
#include "Renderer/GBuffer.h"
#include "Physics/Ragdoll.h"
#include "Physics/PhysX.h"
#include "Core/GameCharacter.h"

class Renderer
{

public: // functions
	
	static void Init(int screenWidth, int screenHeight);
	static void RenderFrame(Camera* camera, int renderWidth, int renderHeight, int player);
	static void DrawGrid(glm::vec3 cameraPos, bool renderFog);
	static void RenderPlayerCrosshair(int renderWidth, int renderHeight, Player* player);

	static void GeometryPass(int player);
	static void LightingPass(int player, int renderWidth, int renderHeight);
	static void DrawScene(Shader* shader, int player);

	static void TextBlitterPass(Shader* shader);
	//static void DrawLine(Shader* shader, Line line, glm::mat4 modelMatrix);
	static void HotLoadShaders();
	//static void DrawTangentDebugAxis(Shader* shader, glm::mat4 modelMatrix, float lineLength);
	//static void DrawAnimatedEntityDebugBones_Animated(Shader* shader, Ragdoll* ragdoll);	
	

	static void DrawLine(Shader* shader, glm::vec3 p1, glm::vec3 p2, glm::vec3 color);
	static void DrawPoint(Shader* shader, glm::vec3 position, glm::vec3 color);
	static void DrawPoint(Shader* shader, glm::mat4 modelMatrix, glm::vec3 color);
	//static void NewGameCharacter();
	//static void RenderJointAxis(Shader*, GameCharacter* gameCharacter);

	static void DrawPhysicsWorld(int player);
	static void ReconfigureFrameBuffers(int height, int width);
	//Shader* GetShader(const char* name);

	static void DrawFullScreenQuad(Shader* shader);
	static void Draw_2_Player_Split_Screen_Quad_Top(Shader* shader);
	static void Draw_2_Player_Split_Screen_Quad_Bottom(Shader* shader);
	static void ClearFBOs(int screenWidth, int screenHeight);

	static void DrawQuad(Shader* shader, glm::mat4 modelMatrix, GLuint texID);

	static void RenderDebugShit();


public: // variables

	static Shader s_geometry_shader;
	static Shader s_solid_color_shader;
	static Shader s_textued_2D_quad_shader;
	static Shader s_lighting_shader;
	static Shader s_final_pass_shader;
	static Shader s_solid_color_shader_editor;
	//static Shader s_textured_editor_shader;

	static bool s_showBuffers;
	static GBuffer s_gBuffer;

	static unsigned int m_uboMatrices;

	PhysX* p_physX;
};

