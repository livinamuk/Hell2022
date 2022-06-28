#include "HellEngine.h"
#include "Core/Input.h"
#include "Audio/Audio.h"
#include "Core/GameData.h"
#include "Core/File.h"
#include "Core/Controller.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_glfw.h"

void HellEngine::Init()
{
	CoreGL::InitGL(SCR_WIDTH, SCR_HEIGHT);
	glPointSize(5);

	PhysX::p_PhysX = &m_physx;

	//Input::Init();

	m_physx.Init();
	Renderer::Init(CoreGL::s_fullscreenWidth, CoreGL::s_fullscreenHeight);

	AssetManager::DiscoverAssetFilenames();
	
	CheckForControllers();

	// Set player 1 and 2 to controllers if they were found.
	if (GameData::s_controllers.size() > 0)
		GameData::s_player2.SetControllerIndex(0);
	if (GameData::s_controllers.size() > 1)
		GameData::s_player1.SetControllerIndex(1);
	if (GameData::s_controllers.size() > 2)
		GameData::s_player3.SetControllerIndex(2);
	if (GameData::s_controllers.size() > 3)
		GameData::s_player4.SetControllerIndex(3);

	// Assign keyboard and mouse
	if (GameData::s_player1.m_inputType == InputType::KEYBOARD_AND_MOUSE) {
		GameData::s_player1.m_mouseIndex = 0;
		GameData::s_player1.m_keyboardIndex = 0;
	}
	if (GameData::s_player2.m_inputType == InputType::KEYBOARD_AND_MOUSE) {
		GameData::s_player2.m_mouseIndex = 1;
		GameData::s_player2.m_keyboardIndex = 1;
	}
	if (GameData::s_player3.m_inputType == InputType::KEYBOARD_AND_MOUSE) {
		GameData::s_player3.m_mouseIndex = 0;
		GameData::s_player3.m_keyboardIndex = 0;
	}
	if (GameData::s_player4.m_inputType == InputType::KEYBOARD_AND_MOUSE) {
		GameData::s_player4.m_mouseIndex = 0;
		GameData::s_player4.m_keyboardIndex = 0;
	}

	// ImGUI
	const char* glsl_version = "#version 150";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(CoreGL::s_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui::GetIO().FontGlobalScale = 3;
}

void HellEngine::Update(float deltaTime)
{
	// dual keyboard input shit
	/*MSG msg;
	//while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	*/

	m_physx.StepPhysics(deltaTime);

	if (Input::KeyPressed(HELL_KEY_C))
		m_switchToPlayer2 = !m_switchToPlayer2;

	if (Input::KeyPressed(HELL_KEY_BACKSPACE))
		Scene::RemoveCorpse();

	GameData::s_player1.m_enableControl = true;

	/*if (GameData::s_controllers.size() == 0 &&
		GameData::s_player1.m_mouseIndex != GameData::s_player2.m_mouseIndex &&
		GameData::s_player1.m_keyboardIndex != GameData::s_player2.m_keyboardIndex) {
		GameData::s_player1.m_enableControl = true;
		GameData::s_player2.m_enableControl = true;
		if (m_switchToPlayer2)
			GameData::s_player1.m_enableControl = false;
		else
			GameData::s_player2.m_enableControl = false;
	}*/

	Input::Update();



	GameData::s_player1.Update(deltaTime);
	GameData::s_player1.m_HUD_Weapon.UpdateAnmation(deltaTime);
	GameData::s_player1.UpdatePlayerModelAnimation(deltaTime);

	if (GameData::s_playerCount > 1) {
		GameData::s_player2.m_enableControl = true;
		GameData::s_player2.Update(deltaTime);
		GameData::s_player2.m_HUD_Weapon.UpdateAnmation(deltaTime);
		GameData::s_player2.UpdatePlayerModelAnimation(deltaTime);
	}

	if (GameData::s_playerCount > 2) {
		GameData::s_player3.m_enableControl = true;
		GameData::s_player3.Update(deltaTime);
		GameData::s_player3.m_HUD_Weapon.UpdateAnmation(deltaTime);
		GameData::s_player3.UpdatePlayerModelAnimation(deltaTime);
	}

	if (GameData::s_playerCount > 3) {
		GameData::s_player4.m_enableControl = true;
		GameData::s_player4.Update(deltaTime);
		GameData::s_player4.m_HUD_Weapon.UpdateAnmation(deltaTime);
		GameData::s_player4.UpdatePlayerModelAnimation(deltaTime);
	}

	
	Input::ResetMouseOffsets();
		
	// Update game character logic
	for (GameCharacter& gameChar : Scene::s_gameCharacters)
			gameChar.Update(deltaTime);
	
	// Animate game characters
	for (GameCharacter& gameChar : Scene::s_gameCharacters)
	{
		if (gameChar.m_skinningMethod == SkinningMethod::ANIMATED)
			gameChar.UpdateAnimation(deltaTime);
	}
	

	// Animate blood pools
	for (BloodPool& bloodPool : Scene::s_bloodPools) {
		bloodPool.Update(deltaTime);
	}

	GameData::Update(deltaTime);

    //if (Input::s_keyPressed[HELL_KEY_P])



	   ProcessCollisions();


}

void HellEngine::UpdateInput()
{
	Input::UpdateKeyboardInput(CoreGL::s_window);
	Input::UpdateMouseInput(CoreGL::s_window);

	GameData::s_player1.UpdateControllerInput();
	GameData::s_player2.UpdateControllerInput();
	GameData::s_player3.UpdateControllerInput();
	GameData::s_player4.UpdateControllerInput();

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
		Renderer::ReconfigureFrameBuffers(CoreGL::s_currentWidth, CoreGL::s_currentHeight);
		Input::m_disableMouseLookTimer = 10;
	}        
	
	if (Input::KeyPressed(HELL_KEY_H))
		Renderer::HotLoadShaders();

	//if (Input::KeyPressed(HELL_KEY_SPACE))
	//	Scene::NewRagdoll();

	/*if (Input::KeyPressed(HELL_KEY_N))
	{
		if (!GameData::s_player1.m_isAlive)
			GameData::s_player1.Respawn();

		if (!GameData::s_player2.m_isAlive)
			GameData::s_player2.Respawn();
	}*/


	/*if (Input::KeyPressed(HELL_KEY_1))
		m_currentPlayer = 1;
	if (Input::KeyPressed(HELL_KEY_2))
		m_currentPlayer = 2;*/

	if (Input::KeyPressed(HELL_KEY_T)) {
		GameData::s_splitScreen = !GameData::s_splitScreen;
		Renderer::ReconfigureFrameBuffers(CoreGL::s_currentWidth, CoreGL::s_currentHeight);
	}


}

void HellEngine::Render()
{
	// Clear the FBOs
	Renderer::ClearFBOs(CoreGL::s_currentWidth, CoreGL::s_currentHeight);

	// Set default render size
	int renderWidth = CoreGL::s_currentWidth;
	int renderHeight = CoreGL::s_currentHeight;

	// If split screen then modify that
	if (GameData::s_splitScreen && GameData::s_playerCount == 2) {
		renderWidth = CoreGL::s_currentWidth;
		renderHeight = CoreGL::s_currentHeight / 2;
		glViewport(0, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth, CoreGL::s_currentHeight / 2);
	}
	else if (GameData::s_splitScreen && GameData::s_playerCount > 2) {
		renderWidth = CoreGL::s_currentWidth / 2;
		renderHeight = CoreGL::s_currentHeight / 2;
		glViewport(0, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
	}


	// Player 1
	GameData::s_player1.UpdateCamera(renderWidth, renderHeight);
	GameData::s_player1.CalculateViewMatrices();
		
	// Matrices
	glBindBuffer(GL_UNIFORM_BUFFER, Renderer::m_uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 0, sizeof(glm::mat4), glm::value_ptr(GameData::s_player1.GetProjectionMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 1, sizeof(glm::mat4), glm::value_ptr(GameData::s_player1.GetViewMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(GameData::s_player1.GetInverseProjectionMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, sizeof(glm::mat4), glm::value_ptr(GameData::s_player1.GetInverseViewMatrix()));
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




	//GameData::s_player1.SpawnGlockCasing();

	Renderer::RenderFrame(GameData::s_player1.GetCameraPointer(), renderWidth, renderHeight, 1);

	// Player 2 
	if (GameData::s_splitScreen)
	{
		glViewport(0, 0, CoreGL::s_currentWidth, CoreGL::s_currentHeight / 2);

		GameData::s_player2.UpdateCamera(renderWidth, renderHeight);
		GameData::s_player2.CalculateViewMatrices();;

		glBindBuffer(GL_UNIFORM_BUFFER, Renderer::m_uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 0, sizeof(glm::mat4), glm::value_ptr(GameData::s_player2.GetProjectionMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 1, sizeof(glm::mat4), glm::value_ptr(GameData::s_player2.GetViewMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4), glm::value_ptr(GameData::s_player2.GetInverseProjectionMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, sizeof(glm::mat4), glm::value_ptr(GameData::s_player2.GetInverseViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		Renderer::RenderFrame(GameData::s_player2.GetCameraPointer(), renderWidth, renderHeight, 2);
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

	//buffer0 = Renderer::s_BlurBuffers_p2[1].textureA;
	//buffer1 = Renderer::s_BlurBuffers_p2[3].textureA;
	//buffer2 = Renderer::s_gBuffer.gFinal;
	//buffer3 = Renderer::s_gBuffer.gPostProcessed;

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
		shader->setMat4("projection", GameData::s_player1.GetProjectionMatrix());
		shader->setMat4("view", GameData::s_player1.GetViewMatrix());
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
		shader->setMat4("projection", GameData::s_player1.GetProjectionMatrix());
		shader->setMat4("view", GameData::s_player1.GetViewMatrix());
		Transform trans;
		trans.position = GameData::s_player1.GetPosition() + glm::vec3(0, GameData::s_player1.m_cameraViewHeight, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, GameData::s_lights[0].m_indirectShadowMap.m_depthTexture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, GameData::s_lights[0].m_envMap.m_TexID);

		AssetManager::m_models["Cube"].Draw(shader, trans.to_mat4());
	}

	// Text blitter UI
	Renderer::TextBlitterPass(&Renderer::s_textued_2D_quad_shader);



	if (Input::s_showCursor) 
	{
		// feed inputs to dear imgui, start new frame
		//ImGui::GetIO().FontGlobalScale = CoreGL::s_currentWidth / CoreGL::s_fullscreenWidth * 2;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Demo window");

		if (ImGui::Button("Clear decals")) {
			GameData::s_bloodDecals.clear();
			GameData::s_bulletDecals.clear();
		}
		if (ImGui::Button("Clear bodies")) {
			Scene::s_gameCharacters.clear();
		}

		ImGui::Checkbox("Show bullet decal positions", &Renderer::s_showBulletDecalPositions);

		ImGui::End();
		// Render dear imgui into screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

void HellEngine::ProcessCollisions()
{
	//return;

	for (CollisionReport& report : ContactReportCallback::s_collisionReports)
	{
		if (!report.dataA)
			continue;
		if (!report.dataB)
			continue;

		// Casings
		if (report.dataA->type == PhysicsObjectType::SHELL_PROJECTILE && report.dataB->type == PhysicsObjectType::FLOOR) {
			BulletCasing* casing = (BulletCasing*)report.dataA->parent;
			casing->m_hitFloor = true;
		//	std::cout << "collision a " << casing->m_hitFloor  << "\n";
		}
		if (report.dataA->type == PhysicsObjectType::SHELL_PROJECTILE && report.dataB->type == PhysicsObjectType::WALL) {
			BulletCasing* casing = (BulletCasing*)report.dataA->parent;
			casing->m_hitWall = true;
		//	std::cout << "collision b \n";
		}
		if (report.dataB->type == PhysicsObjectType::SHELL_PROJECTILE && report.dataA->type == PhysicsObjectType::FLOOR) {
			BulletCasing* casing = (BulletCasing*)report.dataB->parent;
			casing->m_hitFloor = true;
		//	std::cout << "collision c \n";
		}
		if (report.dataB->type == PhysicsObjectType::SHELL_PROJECTILE && report.dataA->type == PhysicsObjectType::WALL) {
			BulletCasing* casing = (BulletCasing*)report.dataB->parent;
			casing->m_hitWall = true;
		//	std::cout << "collision d \n";
		}

		// Game character ragdoll
		/*if (report.dataA->type == PhysicsObjectType::RAGDOLL && report.dataB->type == PhysicsObjectType::FLOOR)
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
		
	}*/
	}
	ContactReportCallback::s_collisionReports.clear();
}

void HellEngine::CheckForControllers()
{
	GameData::s_controllers.clear();

	// Search for 16 controllers lol
	for (int i = 0; i < 16; i++)
	{
		if (glfwJoystickIsGamepad(i))
		{
			std::string joystickName = glfwGetJoystickName(i);
			std::string gamepadName = glfwGetGamepadName(i);

			if (joystickName == "Xbox Controller" || gamepadName == "Xbox Controller") {
				Controller controller;
				controller.m_glfwIndex = i;
				controller.m_type = Controller::Type::XBOX;
				GameData::s_controllers.push_back(controller);
				std::cout << "Controller " << i << " found: XBOX Controller\n";
			}
			else if (joystickName == "PS4 Controller" || gamepadName == "PS4 Controller") {
				Controller controller;
				controller.m_glfwIndex = i;
				controller.m_type = Controller::Type::PS4;
				GameData::s_controllers.push_back(controller);
				std::cout << "Controller " << i << " found: PS4 Controller\n";
			}
			else
				std::cout << "Controller " << i << " unknown controller\n";
		}
	}
}

