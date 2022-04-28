#include "HellEngine.h"
#include "Core/Input.h"
#include "Audio/Audio.h"
#include "Core/GameData.h"
#include "Core/File.h"

void HellEngine::Init()
{
	CoreGL::InitGL(SCR_WIDTH, SCR_HEIGHT);
	glPointSize(5);

	PhysX::p_PhysX = &m_physx;

	m_physx.Init();
	m_renderer.Init(CoreGL::s_fullscreenWidth, CoreGL::s_fullscreenHeight);

	AssetManager::DiscoverAssetFilenames();

	

	GameData::s_player1.p_gameCharacters = &Scene::s_gameCharacters;
	GameData::s_player2.p_gameCharacters = &Scene::s_gameCharacters;
	GameData::s_player1.p_bloodPools = &Scene::s_bloodPools;
	GameData::s_player2.p_bloodPools = &Scene::s_bloodPools;
	

	m_renderer.p_physX = &m_physx;



	/*std::string line, text;
	std::ifstream in("res/gamecontrollerdb.txt");
	while (std::getline(in, line))
	{
		text += line + "\n";
		//std::cout << line << std::endl;
	}
	const char* mappings = text.c_str();
	//glfwUpdateGamepadMappings(mappings);*/


	// Search for 16 controllers lol
	for (int i = 0; i < 16; i++)
	{
		if (glfwJoystickIsGamepad(i)) {
			std::cout << "CONTROLLER " << (i + 1) << " FOUND: " << glfwGetJoystickName(i) << "\n";
			std::cout << "CONTROLLER " << (i + 1) << " FOUND: " << glfwGetGamepadName(i) << "\n"; 

			Controller controller;
			controller.m_index = i;
			controller.m_type = ControllerType::UNKNOWN_TYPE;

			m_controllers.push_back(controller);
		}
	}

	/*
	
	CONTROLLER 1 FOUND: Xbox Controller
	CONTROLLER 1 FOUND: XInput Gamepad (GLFW)
	CONTROLLER 2 FOUND: Wireless Controller
	CONTROLLER 2 FOUND: PS4 Controller
	
	*/
	
	if (m_controllers.size() > 0)
		GameData::s_player2.SetControllerIndex(m_controllers[0].m_index);
	if (m_controllers.size() > 1)
		GameData::s_player1.SetControllerIndex(m_controllers[1].m_index);

	std::cout << "CONTROLLERS FOUND: " << m_controllers.size() << "\n";

}

void HellEngine::Update(float deltaTime)
{
	m_physx.StepPhysics(deltaTime);

	if (Input::KeyPressed(HELL_KEY_C))
		m_switchToPlayer2 = !m_switchToPlayer2;

	if (Input::KeyPressed(HELL_KEY_BACKSPACE))
		Scene::RemoveCorpse();

	if (m_controllers.size() == 0) {
		GameData::s_player1.m_enableControl = true;
		GameData::s_player2.m_enableControl = true;
		if (m_switchToPlayer2)
			GameData::s_player1.m_enableControl = false;
		else
			GameData::s_player2.m_enableControl = false;
	}


	GameData::s_player1.Update(deltaTime);
	GameData::s_player2.Update(deltaTime);
		
	// Update game character logic
	for (GameCharacter& gameChar : Scene::s_gameCharacters)
			gameChar.Update(deltaTime);
	
	// Animate game characters
	for (GameCharacter& gameChar : Scene::s_gameCharacters)
	{
		if (gameChar.m_skinningMethod == SkinningMethod::ANIMATED)
			gameChar.UpdateAnimation(deltaTime);
	}
	
	GameData::s_player1.m_HUD_Weapon.UpdateAnmation(deltaTime);
	GameData::s_player2.m_HUD_Weapon.UpdateAnmation(deltaTime);

	// Animate blood pools
	for (BloodPool& bloodPool : Scene::s_bloodPools) {
		bloodPool.Update(deltaTime);
	}

	GameData::Update(deltaTime);

    //if (Input::s_keyPressed[HELL_KEY_P])

	   GameData::s_player1.UpdatePlayerModelAnimation(deltaTime);
	   GameData::s_player2.UpdatePlayerModelAnimation(deltaTime);

	   ProcessCollisions();


}

void HellEngine::UpdateInput()
{
	Input::UpdateKeyboardInput(CoreGL::s_window);
	Input::UpdateMouseInput(CoreGL::s_window);
	Input::UpdateControllerInput(GameData::s_player1.GetControllerIndex());
	Input::UpdateControllerInput(GameData::s_player2.GetControllerIndex());

	CheckForDebugKeyPresses();
}

void HellEngine::CheckForDebugKeyPresses()
{

	if (Input::KeyPressed(HELL_KEY_TAB)) {
		Editor::ToggleEditor();
		Audio::PlayAudio("UI_Select2.wav", 0.5f);
	}	
	
	if (Input::KeyPressed(HELL_KEY_CAPS_LOCK)) {
		Renderer::s_showBuffers = !Renderer::s_showBuffers;
	}

	//if (Input::KeyPressed(HELL_KEY_G))
	//	File::LoadMap("Map.json");

	if (Input::KeyPressed(HELL_KEY_F)) {
		CoreGL::ToggleFullScreen();
		m_renderer.ReconfigureFrameBuffers(CoreGL::s_currentWidth, CoreGL::s_currentHeight);
		Input::m_disableMouseLookTimer = 10;
	}        
	
	if (Input::KeyPressed(HELL_KEY_H))
		m_renderer.HotLoadShaders();

	//if (Input::KeyPressed(HELL_KEY_SPACE))
	//	Scene::NewRagdoll();

	if (Input::KeyPressed(HELL_KEY_N)) 
	{
		if (!GameData::s_player1.m_isAlive)
			GameData::s_player1.Respawn();

		if (!GameData::s_player2.m_isAlive)
			GameData::s_player2.Respawn();
	}


	if (Input::KeyPressed(HELL_KEY_1))
		m_currentPlayer = 1;
	if (Input::KeyPressed(HELL_KEY_2))
		m_currentPlayer = 2;

	if (Input::KeyPressed(HELL_KEY_T))
		GameData::s_splitScreen = !GameData::s_splitScreen;


}

void HellEngine::Render()
{
	// Clear the FBOs
	m_renderer.ClearFBOs(CoreGL::s_currentWidth, CoreGL::s_currentHeight);

	// Set default render size
	int renderWidth = CoreGL::s_currentWidth;
	int renderHeight = CoreGL::s_currentHeight;

	// If split screen then modify that
	if (GameData::s_splitScreen) {
		renderWidth = CoreGL::s_currentWidth;
		renderHeight = CoreGL::s_currentHeight / 2;
		glViewport(0, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth, CoreGL::s_currentHeight/2);
	}

	// Player 1
	GameData::s_player1.UpdateCamera(renderWidth, renderHeight);
	
	// Matrices
	glBindBuffer(GL_UNIFORM_BUFFER, Renderer::m_uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 0, sizeof(glm::mat4), glm::value_ptr(GameData::s_player1.GetCameraProjectionMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 1, sizeof(glm::mat4), glm::value_ptr(GameData::s_player1.GetCameraViewMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, sizeof(glm::mat4), glm::value_ptr(glm::inverse(GameData::s_player1.GetCameraProjectionMatrix())));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 4, sizeof(glm::mat4), glm::value_ptr(glm::inverse(GameData::s_player1.GetCameraViewMatrix())));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Lights
	glBindBuffer(GL_UNIFORM_BUFFER, Renderer::m_uboLights);
	for (int i = 0; i < GameData::s_lights.size() && i < MAX_LIGHTS; i++) 
	{
		Light* light = &GameData::s_lights[i];	
		glm::vec4 pos = glm::vec4(light->m_position, light->m_radius);
		glm::vec4 color = glm::vec4(light->m_color, light->m_strength);
		glm::vec4 magic = glm::vec4(light->m_radius, 0, 0, 0);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(LightUniformBlock) * i + sizeof(glm::vec4) * 0, sizeof(glm::vec4), glm::value_ptr(pos));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(LightUniformBlock) * i + sizeof(glm::vec4) * 1, sizeof(glm::vec4), glm::value_ptr(color));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(LightUniformBlock) * i + sizeof(glm::vec4) * 2, sizeof(glm::vec4), glm::value_ptr(magic));
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);




	GameData::s_player1.SpawnGlockCasing();

	m_renderer.RenderFrame(GameData::s_player1.GetCameraPointer(), renderWidth, renderHeight, 1);

	// Player 2 
	if (GameData::s_splitScreen)
	{
		glViewport(0, 0, CoreGL::s_currentWidth, CoreGL::s_currentHeight / 2);

		GameData::s_player2.UpdateCamera(renderWidth, renderHeight);

		glBindBuffer(GL_UNIFORM_BUFFER, Renderer::m_uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 0, sizeof(glm::mat4), glm::value_ptr(GameData::s_player2.GetCameraProjectionMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 1, sizeof(glm::mat4), glm::value_ptr(GameData::s_player2.GetCameraViewMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(glm::inverse(GameData::s_player2.GetCameraProjectionMatrix())));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, sizeof(glm::mat4), glm::value_ptr(glm::inverse(GameData::s_player2.GetCameraViewMatrix())));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		m_renderer.RenderFrame(GameData::s_player2.GetCameraPointer(), renderWidth, renderHeight, 2);
	}

	// Render final image to default frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Renderer::s_gBuffer.gFinal);
	//glBindTexture(GL_TEXTURE_2D, m_renderer.s_gBuffer.gAlbedo);
	glViewport(0, 0, CoreGL::s_currentWidth, CoreGL::s_currentHeight);
	//Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);
	Renderer::DrawViewportQuad(&Renderer::s_textued_2D_quad_shader, Renderer::ViewportSize::FULL_SCREEN);

	GLint buffer0 = Renderer::s_gBuffer.gAlbedo;
	GLint buffer1 = Renderer::s_gBuffer.gNormal;
	GLint buffer2 = Renderer::s_gBuffer.gFinal;
	GLint buffer3 = Renderer::s_gBuffer.gRMA;

	if (Renderer::s_showBuffers)
	{
		glViewport(0, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buffer0);
		//Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);
		Renderer::DrawViewportQuad(&Renderer::s_textued_2D_quad_shader, Renderer::ViewportSize::FULL_SCREEN);

		glViewport(CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buffer1);
		Renderer::DrawViewportQuad(&Renderer::s_textued_2D_quad_shader, Renderer::ViewportSize::FULL_SCREEN);
		//Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);

		glViewport(0, 0, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buffer2);
		Renderer::DrawViewportQuad(&Renderer::s_textued_2D_quad_shader, Renderer::ViewportSize::FULL_SCREEN);
		//Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);

		glViewport(CoreGL::s_currentWidth / 2, 0, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buffer3);
		//glBindTexture(GL_TEXTURE_2D, GameData::s_lights[0].m_envMap.SH_TexID);
		Renderer::DrawViewportQuad(&Renderer::s_textued_2D_quad_shader, Renderer::ViewportSize::FULL_SCREEN); 
		//Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);
	}

	// previouw shadow map
	if (Input::KeyDown(HELL_KEY_Z))
	{
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, CoreGL::s_currentWidth, CoreGL::s_currentHeight);
		Shader* shader = &Renderer::s_skybox_shader;
		shader->use();
		shader->setMat4("projection", GameData::s_player1.GetCameraProjectionMatrix());
		shader->setMat4("view", GameData::s_player1.GetCameraViewMatrix());
		Transform trans;
		trans.position = GameData::s_player1.GetPosition() + glm::vec3(0, GameData::s_player1.m_cameraViewHeight, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, GameData::s_lights[0].m_shadowMap.m_depthTexture);

		AssetManager::m_models["Cube"].Draw(shader, trans.to_mat4());
	}

	if (Input::KeyDown(HELL_KEY_X))
	{
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, CoreGL::s_currentWidth, CoreGL::s_currentHeight);
		Shader* shader = &Renderer::s_skybox_shader;
		shader->use();
		shader->setMat4("projection", GameData::s_player1.GetCameraProjectionMatrix());
		shader->setMat4("view", GameData::s_player1.GetCameraViewMatrix());
		Transform trans;
		trans.position = GameData::s_player1.GetPosition() + glm::vec3(0, GameData::s_player1.m_cameraViewHeight, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, GameData::s_lights[0].m_indirectShadowMap.m_depthTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, GameData::s_lights[0].m_envMap.m_TexID);

		AssetManager::m_models["Cube"].Draw(shader, trans.to_mat4());
	}

	// Text blitter UI
	Renderer::TextBlitterPass(&Renderer::s_textued_2D_quad_shader);
}

void HellEngine::ProcessCollisions()
{
	return;

	for (CollisionReport& report : ContactReportCallback::s_collisionReports)
	{
		if (!report.dataA)
			continue;
		if (!report.dataB)
			continue;

		// Game character ragdoll
		if (report.dataA->type == PhysicsObjectType::RAGDOLL && report.dataB->type == PhysicsObjectType::FLOOR)
		{
			PxRigidDynamic* rigid = (PxRigidDynamic*)report.rigidA;
			if (!rigid)
				continue;

			//std::cout << rigid->getLinearVelocity().x << ", " << rigid->getLinearVelocity().y << ", " << rigid->getLinearVelocity().z << "\n";

			float velocity = rigid->getLinearVelocity().magnitude();

			if (velocity > 1.9f) {
				GameCharacter* gameCharacter = (GameCharacter*)report.dataA->parent;
				gameCharacter->HitFloor();
			}
		}
		// Player ragdoll
		else if (report.dataA->type == PhysicsObjectType::PLAYER_RAGDOLL && report.dataB->type == PhysicsObjectType::FLOOR)
		{
			Player* player = (Player*)report.dataA->parent;
			PxRigidDynamic* rigid = (PxRigidDynamic*)report.rigidA;
			float velocity = rigid->getLinearVelocity().magnitude();

			if (velocity > 1.9f) {
				player->HitFloor();
			}

			//if (rigid->getName() == "RAGDOLL_HEAD")
			//	player->SpawnBloodPool();
		}
	}

	ContactReportCallback::s_collisionReports.clear();
}


