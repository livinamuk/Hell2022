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
			std::cout << "CONTROLLER " << (i+1) << " FOUND: " << glfwGetGamepadName(i) << "\n";
			m_controllers.push_back(i);
		}
	}

	
	if (m_controllers.size() > 0)
		GameData::s_player2.SetControllerIndex(m_controllers[0]);
	if (m_controllers.size() > 1)
		GameData::s_player1.SetControllerIndex(m_controllers[1]);

	std::cout << "CONTROLLERS FOUND: " << m_controllers.size() << "\n";

}

void HellEngine::Update(float deltaTime)
{
	if (Input::KeyPressed(HELL_KEY_C))
		m_switchToPlayer2 = !m_switchToPlayer2;

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
       m_physx.StepPhysics();

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
	
	glBindBuffer(GL_UNIFORM_BUFFER, Renderer::m_uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 0, sizeof(glm::mat4), glm::value_ptr(GameData::s_player1.GetCameraProjectionMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 1, sizeof(glm::mat4), glm::value_ptr(GameData::s_player1.GetCameraViewMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 3, sizeof(glm::mat4), glm::value_ptr(glm::inverse(GameData::s_player1.GetCameraProjectionMatrix())));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 4, sizeof(glm::mat4), glm::value_ptr(glm::inverse(GameData::s_player1.GetCameraViewMatrix())));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);




	GameData::s_player1.SpawnGlockCasing();

	m_renderer.RenderFrame(GameData::s_player1.GetCameraPointer(), renderWidth, renderHeight, 1);


	/*if (!Input::s_keyDown[HELL_KEY_C])
		m_player2.m_isAlive = true;
	else
		m_player2.m_isAlive = false;*/

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
	Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);


	if (Renderer::s_showBuffers)
	{
		glViewport(0, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Renderer::s_gBuffer.gAlbedo);
		Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);

		glViewport(CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Renderer::s_gBuffer.gNormal);
		Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);

		glViewport(0, 0, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Renderer::s_gBuffer.gRMA);
		Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);

		glViewport(CoreGL::s_currentWidth / 2, 0, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Renderer::s_gBuffer.gLighting);
		Renderer::DrawFullScreenQuad(&Renderer::s_textued_2D_quad_shader);
	}

	// Text blitter UI
	Renderer::TextBlitterPass(&Renderer::s_textued_2D_quad_shader);
}

void HellEngine::ProcessCollisions()
{
	for (CollisionReport& report : ContactReportCallback::s_collisionReports)
	{
		// Game character ragdoll
		if (report.dataA->type == PhysicsObjectType::RAGDOLL && report.dataB->type == PhysicsObjectType::FLOOR)
		{
			PxRigidDynamic* rigid = (PxRigidDynamic*)report.rigidA;
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


