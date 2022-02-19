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
/*	struct ModelPtrs {
		Model* p_CubeLines;
		Model* p_Sphere;
		Model* p_SphereLines;
		Model* p_UncappedCylinder;
		Model* p_HalfSphere2;
		Model* p_HalfSphere;
		Model* p_UncappedCylinderLines;
		Model* p_HalfSphereLines;
		Model* p_HalfSphereLines2;		
	};*/

public: // functions
	
	void Init(int screenWidth, int screenHeight);
	//void SetModelPointers();
	void RenderFrame(Scene* scene, Camera* camera, int renderWidth, int renderHeight, int player, bool splitscreen);
	void DrawGrid(Camera* camera);
	void RenderPlayerCrosshair(int renderWidth, int renderHeight, Player* player);

	void RenderFinalImage();
	void TextBlitterPass(Shader* shader);
	void DrawLine(Shader* shader, Line line, glm::mat4 modelMatrix);
	void HotLoadShaders();
	//static void DrawTangentDebugAxis(Shader* shader, glm::mat4 modelMatrix, float lineLength);
	//static void DrawAnimatedEntityDebugBones_Animated(Shader* shader, Ragdoll* ragdoll);
	void DrawPoint(Shader* shader, glm::vec3 position, glm::vec3 color);
	//static void NewGameCharacter();
	//static void RenderJointAxis(Shader*, GameCharacter* gameCharacter);

	void DrawPhysicsWorld(int player);
	void ReconfigureFrameBuffers(int height, int width);
	//Shader* GetShader(const char* name);

	void DrawFullScreenQuad(Shader* shader);
	void Draw_2_Player_Split_Screen_Quad_Top(Shader* shader);
	void Draw_2_Player_Split_Screen_Quad_Bottom(Shader* shader);
	void ClearFBOs(int screenWidth, int screenHeight);

	void DrawQuad(Shader* shader, glm::mat4 modelMatrix, GLuint texID);

public: // variables


	Shader m_test_shader;
	Shader m_solid_color_shader;
	Shader m_textued_2D_quad_shader;
	Shader m_skinned_model_shader;
	Shader m_final_pass_shader;

	GBuffer s_gBuffer;

	PhysX* p_physX;
	//std::unordered_map<std::string, Model>* p_models;

	//ModelPtrs m_modelPointers;

	unsigned int m_uboMatrices;

	Player* p_player1;
	Player* p_player2;

	//int m_screenWidth;
	//int m_screenHeight;


};

