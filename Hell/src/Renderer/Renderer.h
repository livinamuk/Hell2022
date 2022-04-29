#pragma once
#include "Renderer/Decal.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/Model.h"
#include "Renderer/BlurBuffer.h"
#include "Core/Camera.h"
#include "Core/Player.h"
#include "Core/CoreGL.h"
#include "Core/Entity.h"
#include "Core/Scene.h"
#include "Renderer/GBuffer.h"
#include "Physics/Ragdoll.h"
#include "Physics/PhysX.h"
#include "Core/GameCharacter.h"
#include "Effects/MuzzleFlash.h"

class Renderer
{
public: // functions

	enum class ViewportSize {
		FULL_SCREEN,
		SPLITSCREEN_TOP,
		SPLITSCREEN_BOTTOM
	};

	enum class RenderPass {
		GEOMETRY,
		ENV_MAP,
		SHADOW_MAP,
		INDIRECT_SHADOW_MAP
	};

	static void Init(int screenWidth, int screenHeight);
	static void RenderFrame(Camera* camera, int renderWidth, int renderHeight, int player);
	static void DrawGrid(glm::vec3 cameraPos, bool renderFog);
	static void RenderPlayerCrosshair(int renderWidth, int renderHeight, Player* player);

	static void EnvMapPass();
	static void IndirectShadowMapPass();
	static void ShadowMapPass();
	static void GeometryPass(int player, int renderWidth, int renderHeight);
	static void DecalPass(int player, int renderWidth, int renderHeight);
	static void LightingPass(int player, int renderWidth, int renderHeight);
	static void MuzzleFlashPass(int player, int renderWidth, int renderHeight);
	static void PostProcessingPass(int player);
	static void EmissiveBlurPass(int player, int renderWidth, int renderHeight, int levels);


	static void DrawScene(Shader* shader, RenderPass renderPass, int player = 0);

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

	//static void DrawFullScreenQuad(Shader* shader);
	//static void Draw_2_Player_Split_Screen_Quad_Top(Shader* shader);
	//static void Draw_2_Player_Split_Screen_Quad_Bottom(Shader* shader);
	static void ClearFBOs(int screenWidth, int screenHeight);

	static void DrawQuad(Shader* shader, glm::mat4 modelMatrix, GLuint texID);

	static void RenderDebugShit();

	static void DrawViewportQuad(Shader* shader, ViewportSize viewportSize);
	static void CreateBRDFLut();
	static void SetViewport(int player);
	static float GetViewportWidth(int player);
	static float GetViewportHeight(int player);
	static ViewportSize GetViewportSize(int player);
	static Player* GetPlayerFromIndex(int index);

public: // variables

	static Shader s_geometry_shader;
	static Shader s_solid_color_shader;
	static Shader s_textued_2D_quad_shader;
	static Shader s_lighting_shader;
	static Shader s_final_pass_shader;
	static Shader s_solid_color_shader_editor;
	static Shader s_shadow_map_shader;
	static Shader s_skybox_shader;
	static Shader s_brdf_shader;
	static Shader s_env_map_shader;
	static Shader s_SH_shader;
	static Shader s_animated_quad_shader;
	static Shader s_postProcessingShader;
	static Shader s_decal_shader;
	static Shader s_blood_decal_shader;
	static Shader s_horizontal_blur_shader;
	static Shader s_vertical_blur_shader;
	//static Shader s_textured_editor_shader;

	static bool s_showBuffers;
	static GBuffer s_gBuffer;
	static BlurBuffer s_BlurBuffers_p1[4];
	static BlurBuffer s_BlurBuffers_p2[4];

	static unsigned int m_uboMatrices;
	static unsigned int m_uboLights;

	PhysX* p_physX;
	static GLuint s_centeredQuadVAO;
	static GLuint s_quadVAO;
	static GLuint s_brdfLUTTextureID;

	static MuzzleFlash s_muzzleFlash;
};

