#include "Player.h"
#include "Helpers/AssetManager.h"
#include "Audio/Audio.h"
#include "Physics/PhysX.h"
#include "GameCharacter.h"
#include "Effects/BloodPool.h"
#include "Core/GameData.h"

void Player::Interact()
{
	if (m_cameraRay.HitFound())
	{
		// Doors
		if (m_cameraRay.m_physicsObjectType == PhysicsObjectType::DOOR) {
			Door* door = (Door*)m_cameraRay.m_parent;
			door->Interact();
		}
	}
}

void Player::UpdateCamera(int renderWidth, int renderHeight)
{
	m_camera.CalculateProjectionMatrix(renderWidth, renderHeight);	
}

void Player::Update(float deltaTime)
{
	if (m_muzzleFlashTimer >= 0)
		m_muzzleFlashTimer += deltaTime * 15;

	// if player is alive
	if (m_isAlive) 
	{
		m_timeSinceDeath = 0;	
		
		if (m_enableControl) {
			UpdateMovement(deltaTime);			
			UpdateAiming();
			CheckForWeaponInput();

			if (PressedInteract())
				Interact();
		}
	}
	// If player is dead
	else
	{
		m_timeSinceDeath += deltaTime;		

		SpawnBloodPool();

		// Presses Respawn
		if (m_enableControl && !m_isAlive && m_timeSinceDeath > 3.25)
		{
			if (PressedFire() ||
				PressedReload() ||
				PressedCrouch() ||
				PressedInteract() ||
				PressedJump()
				)
				Respawn();
		}
	}
		

	FireCameraRay();
}

void Player::Respawn()
{
	m_corpse = nullptr;



	m_ammo_glock_in_clip = m_clip_size_glock;
	m_ammo_shotgun_in_clip = m_clip_size_shotgun;
	m_ammo_glock_total = 80;
	m_ammo_shotgun_total = 48;

	m_isAlive = true;
	m_transform = Transform();
	m_hasHitFloorYet = false;
	//m_transform.position = glm::vec3(Util::RandomFloat(-10, 10), 0, Util::RandomFloat(-10, 10));
	//m_camera.m_transform.rotation = glm::vec3(0, Util::RandomFloat(0, 6), 0);
	m_justBorn = true;
	m_createdBloodPool = false;

	//m_HUDWeaponAnimationState = HUDWeaponAnimationState::EQUIPPING;

	std::string file = "UI_Select2.wav";
	Audio::PlayAudio(file.c_str(), 0.5f);


	int spawnLoc = Util::RandomFloat(0, 9);

	if (spawnLoc == 8) {
		m_transform.position = glm::vec3(3.32548, -0.00066999, 0.162508);
		m_camera.m_transform.rotation = glm::vec3(-0.139529, 1.57592, 0);
	}
	else if (spawnLoc == 7) {
		m_transform.position = glm::vec3(-2.01669, -0.0010103, 7.57015);
		m_camera.m_transform.rotation = glm::vec3(-0.118587, 0.0314156, 0);
	}
	else if (spawnLoc == 2) {
		m_transform.position = glm::vec3(-7.79973, -8.73399e-05, 3.36098);
		m_camera.m_transform.rotation = glm::vec3(-0.170943, -3.09424, 0);
	}
	else if (spawnLoc == 3) {
		m_transform.position = glm::vec3(1.92737, -0.00188446, -1.28305);
		m_camera.m_transform.rotation = glm::vec3(-0.212828, -3.80628, 0);
	}
	else if (spawnLoc == 4) {
		m_transform.position = glm::vec3(4.01202, -0.00145661, 3.09761);
		m_camera.m_transform.rotation = glm::vec3(-0.275655, -4.07853, 0);
	}
	else if (spawnLoc == 5) {
		m_transform.position = glm::vec3(4.14206, -0.000171214, 4.26873);
		m_camera.m_transform.rotation = glm::vec3(-0.139529, -4.73822, 0);
	}
	else if (spawnLoc == 6) {
		m_transform.position = glm::vec3(-1.8943, -0.000382753, -1.32018);
		m_camera.m_transform.rotation = glm::vec3(-0.129058, -2.43979, 0);
	}
	else if (spawnLoc == 0) {
		m_transform.position = glm::vec3(1.92737, -0.00188446, -1.28305);
		m_camera.m_transform.rotation = glm::vec3(-0.212828, -3.80628, 0);
	}
	else if (spawnLoc == 1) {
		m_transform.position = glm::vec3(-7.8822, 0.00285899, -3.36048);
		m_camera.m_transform.rotation = glm::vec3(-0.0916229, -3.14659, 0);
	}

	// Move character controller to spawn point
	glm::vec3 pos = m_transform.position;
	m_characterController->setPosition(PxExtendedVec3(pos.x, pos.y + 0.4, pos.z));
}

void Player::FireCameraRay()
{
	// First omit the player ragdoll from it's own raycast
	for (RigidComponent& rigid : m_ragdoll.m_rigidComponents) {
		PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
		PhysX::DisableRayCastingForShape(shape);
	}

	// Cast ray
	glm::vec3 origin = glm::vec3(m_transform.position.x, m_transform.position.y + m_cameraViewHeight, m_transform.position.z);
	glm::vec3 unitDir = m_camera.m_Front;	
	float distance = 100;
	m_cameraRay.CastRay(origin, unitDir, distance);

	// Renable raycasting for the ragdoll
	for (RigidComponent& rigid : m_ragdoll.m_rigidComponents) {
		PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
		PhysX::EnableRayCastingForShape(shape);
	}
}

void Player::FireBulletRay(float variance)
{
	// First omit the player ragdoll from it's own raycast
	for (RigidComponent& rigid : m_ragdoll.m_rigidComponents) {
		PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
		PhysX::DisableRayCastingForShape(shape);
	}

	// Cast ray
	glm::vec3 origin = glm::vec3(m_transform.position.x, m_transform.position.y + m_cameraViewHeight, m_transform.position.z);
	glm::vec3 unitDir = m_camera.m_Front;

	// Add variance
	unitDir.x += (variance * 0.5f) - Util::RandomFloat(0, variance);
	unitDir.y += (variance * 0.5f) - Util::RandomFloat(0, variance);
	unitDir.z += (variance * 0.5f) - Util::RandomFloat(0, variance);
	unitDir = glm::normalize(unitDir);
	
	float distance = 100;
	m_bulletRay.CastRay(origin, unitDir, distance);

	// Renable raycasting for the ragdoll
	for (RigidComponent& rigid : m_ragdoll.m_rigidComponents) {
		PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
		PhysX::EnableRayCastingForShape(shape);
	}
}

void Player::FireBulletRay(glm::vec3 unitDir)
{
	// First omit the player ragdoll from it's own raycast
	for (RigidComponent& rigid : m_ragdoll.m_rigidComponents) {
		PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
		PhysX::DisableRayCastingForShape(shape);
	}

	// Cast ray
	glm::vec3 origin = glm::vec3(m_transform.position.x, m_transform.position.y + m_cameraViewHeight, m_transform.position.z);

	float distance = 100;
	m_bulletRay.CastRay(origin, unitDir, distance);

	// Renable raycasting for the ragdoll
	for (RigidComponent& rigid : m_ragdoll.m_rigidComponents) {
		PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
		PhysX::EnableRayCastingForShape(shape);
	}
}

#define WALKING_SPEED 3.5f
#define CROUCHING_SPEED 2.125f


void Player::UpdateMovement(float deltaTime)
{
	glm::vec3 Forward = glm::normalize(glm::vec3(m_camera.m_Front.x, 0, m_camera.m_Front.z));

	float speed = 0;
	if (!m_isCrouching)
		speed= WALKING_SPEED;
	else if (m_isCrouching)
		speed = CROUCHING_SPEED;

	m_isMoving = false;

//	const PxExtendedVec3& globalPose = m_characterController->getPosition();


	//m_transform.position = glm::vec3(globalPose.x, globalPose.y - 0.4f, globalPose.z);

	glm::vec3 displacement = glm::vec3(0,0,0);

	// Keyboard / controller button movement
	if (PressingWalkForward()) {
		displacement += Forward;// *deltaTime;
		m_isMoving = true;
	}
	if (PressingWalkBackward()) {
		displacement -= Forward;// *deltaTime;
		m_isMoving = true;
	}
	if (PressingWalkLeft()) {
		displacement -= m_camera.m_Right;// *deltaTime;
		m_isMoving = true;
	}
	if (PressingWalkRight()) {
		displacement += m_camera.m_Right;// *deltaTime;
		m_isMoving = true;
	}

	// walk speed
	if (!m_isCrouching)
		displacement *= 0.0575;
	// crouch speed
	else if (m_isCrouching)
		displacement *= (0.038725);
	
	//deltaTime *= 0.14f;
	displacement *= deltaTime;
	displacement *= 60 / 1;;

	PxFilterData filterData;
	filterData.word1 = PhysX::CollisionGroup::MISC_OBSTACLE;										//bitmask of the object
	//filterData.word2 = PhysX::CollisionGroup::MISC_OBSTACLE;// || PhysX::CollisionGroup::MISC_OBSTACLE; //bitmask for groups it collides with

	PxControllerFilters data;
	data.mFilterData = &filterData;


	if (Input::KeyPressed(HELL_KEY_G)) {
		m_characterController->setPosition(PxExtendedVec3(2, 2, 2));
	}


	PxVec3 disp = PxVec3(displacement.x, -9.8f, displacement.z);
	PxF32 minDist = 0.001;

	m_characterController->move(PxVec3(displacement.x, -9.8f, displacement.z), minDist, deltaTime, data);

	//std::cout << disp.x << " " << disp.y << " " << disp.z << "\n";
	//std::cout << deltaTime << "\n";

	// Controller stick movement
	if (m_controllerIndex != -1)
	{
		float deadZone = 0.25f;
		float walk_trigger_axis_X;
		float walk_trigger_axis_Y;
	
		if (m_leftStickMode == ControllerStickMode::MOVEMENT) {
			walk_trigger_axis_X = Input::s_controllerStates[m_controllerIndex].left_stick_axis_X;
			walk_trigger_axis_Y = Input::s_controllerStates[m_controllerIndex].left_stick_axis_Y;
		}		
		if (m_rightStickMode == ControllerStickMode::MOVEMENT) {
			walk_trigger_axis_X = Input::s_controllerStates[m_controllerIndex].right_stick_axis_X;
			walk_trigger_axis_Y = Input::s_controllerStates[m_controllerIndex].right_stick_axis_Y;
		}

		if (!PressingWalkForward() && (walk_trigger_axis_Y < 0 - deadZone)) {
			m_transform.position += Forward * deltaTime * speed * abs(walk_trigger_axis_Y);
			m_isMoving = true;
		}
		if (!PressingWalkBackward() && (walk_trigger_axis_Y > 0 + deadZone)) {
			m_transform.position -= Forward * deltaTime * speed * abs(walk_trigger_axis_Y);
			m_isMoving = true;
		}
		if (!PressingWalkLeft() && (walk_trigger_axis_X < 0 - deadZone)) {
			m_transform.position -= m_camera.m_Right * deltaTime * speed * abs(walk_trigger_axis_X);
			m_isMoving = true;
		}
		if (!PressingWalkRight() && (walk_trigger_axis_X > 0 + deadZone)) {
			m_transform.position += m_camera.m_Right * deltaTime * speed * abs(walk_trigger_axis_X);
			m_isMoving = true;
		}
	}

	// Crouching
	m_isCrouching = false;
	if (PressingCrouch()) {
		m_isCrouching = true;
	}

	// Update view height
	float target = m_isCrouching ? m_viewHeightCrouching : m_viewHeightStanding;
	m_cameraViewHeight = Util::FInterpTo(m_cameraViewHeight, target, deltaTime, m_crouchDownSpeed);

	// Weapon sway
	float xOffset = 0;
	float yOffset = 0;
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE) {
		xOffset = Input::m_xoffset;
		yOffset = Input::m_yoffset;
		//std::cout << xOffset << "\n";
	}
	else if (m_inputType == InputType::CONTROLLER) {
		xOffset = m_aim_trigger_axis_X * 1000;
		yOffset = m_aim_trigger_axis_Y * 1000;
	}

	if (m_gun == Gun::GLOCK)
		m_camera.CalculateWeaponSway(deltaTime, xOffset, yOffset, 2.25f);

	if (m_gun == Gun::SHOTGUN)
		m_camera.CalculateWeaponSway(deltaTime, xOffset, yOffset, 4.25f);

	FootstepAudio(deltaTime);


	if (Input::KeyPressed(HELL_KEY_H)) {
		std::cout << "player pos: " << m_transform.position.x << ",";
		std::cout << m_transform.position.y << ",";
		std::cout << m_transform.position.z << "\n";
		std::cout << "cam rot: " << m_camera.m_transform.rotation.x << ",";
		std::cout << m_camera.m_transform.rotation.y << ",";
		std::cout << m_camera.m_transform.rotation.z << "\n\n";
	}
}

void Player::FootstepAudio(float deltaTime)
{
	float footstepAudioLoopLength = 0.25f;
	if (IsCrouched())
		footstepAudioLoopLength = 0.35f;

	if (!IsMoving())
		m_footstepAudioTimer = 0;
	else 
	{
		if (IsMoving() && m_footstepAudioTimer == 0) {
			int random_number = std::rand() % 4 + 1;
			std::string file = "player_step_" + std::to_string(random_number) + ".wav";
			Audio::PlayAudio(file.c_str(), 0.5f);
		}
		m_footstepAudioTimer += deltaTime * 0.6;

		///if (!isRunning)
		//	footstepAudioLoopLength = 0.35f;

		if (m_footstepAudioTimer > footstepAudioLoopLength)
			m_footstepAudioTimer = 0;
	}
}

void Player::UpdateAiming()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
	{
		// This is to stop mouse jumping when switching between fullscreen
		if (Input::m_disableMouseLookTimer > 0) {
			Input::m_disableMouseLookTimer--;
			return;
		}

		// Bail if program wants the actual cursor
		if (Input::s_showCursor)
			return;

		// Apply actual trigger position to the camera
		float yLimit = 1.5f;
		m_camera.m_transform.rotation += glm::vec3(-Input::m_yoffset, -Input::m_xoffset, 0.0) / glm::vec3(201 - 10);
		m_camera.m_transform.rotation.x = std::min(m_camera.m_transform.rotation.x, yLimit);
		m_camera.m_transform.rotation.x = std::max(m_camera.m_transform.rotation.x, -yLimit);
	}

	if (m_inputType == InputType::CONTROLLER && m_controllerIndex != -1)
	{
		// Figure out which fucking stick they are using
		m_aim_trigger_axis_X = 0;
		m_aim_trigger_axis_Y = 0;
		if (m_leftStickMode == ControllerStickMode::AIMING) {
			m_aim_trigger_axis_X = Input::s_controllerStates[m_controllerIndex].left_stick_axis_X;
			m_aim_trigger_axis_Y = Input::s_controllerStates[m_controllerIndex].left_stick_axis_Y;
		}
		if (m_rightStickMode == ControllerStickMode::AIMING) {
			m_aim_trigger_axis_X = Input::s_controllerStates[m_controllerIndex].right_stick_axis_X;
			m_aim_trigger_axis_Y = Input::s_controllerStates[m_controllerIndex].right_stick_axis_Y;
		}

		// Prevent drifting
		float deadZone = 0.1f;
		if (m_aim_trigger_axis_X < deadZone && m_aim_trigger_axis_X > -deadZone)
			m_aim_trigger_axis_X = 0;
		if (m_aim_trigger_axis_Y < deadZone && m_aim_trigger_axis_Y > -deadZone)
			m_aim_trigger_axis_Y = 0;

		// Apply sensitivity
		m_aim_trigger_axis_X *= 0.08;
		m_aim_trigger_axis_Y *= 0.08;

		// Apply actual trigger position to the camera
		float yLimit = 1.5f;
		m_camera.m_transform.rotation += glm::vec3(-m_aim_trigger_axis_Y, -m_aim_trigger_axis_X, 0.0);
		m_camera.m_transform.rotation.x = std::min(m_camera.m_transform.rotation.x, yLimit);
		m_camera.m_transform.rotation.x = std::max(m_camera.m_transform.rotation.x, -yLimit);
	}
}



bool Player::PressingWalkForward()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyDown(m_controls.WALK_FORWARD);
	else
		return Input::ButtonDown(m_controllerIndex, m_controls.WALK_FORWARD);
}

bool Player::PressingWalkBackward()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyDown(m_controls.WALK_BACKWARD);
	else
		return Input::ButtonDown(m_controllerIndex, m_controls.WALK_BACKWARD);
}

bool Player::PressingWalkLeft()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyDown(m_controls.WALK_LEFT);
	else
		return Input::ButtonDown(m_controllerIndex, m_controls.WALK_LEFT);
}

bool Player::PressingWalkRight()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyDown(m_controls.WALK_RIGHT);
	else
		return Input::ButtonDown(m_controllerIndex, m_controls.WALK_RIGHT);
}

bool Player::PressingCrouch()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyDown(m_controls.CROUCH);
	else
		return Input::ButtonDown(m_controllerIndex, m_controls.CROUCH);
}

bool Player::PressedInteract()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyPressed(m_controls.INTERACT);
	else
		return Input::ButtonPressed(m_controllerIndex, m_controls.INTERACT);
}

bool Player::PressedReload()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyPressed(m_controls.RELOAD);
	else
		return Input::ButtonPressed(m_controllerIndex, m_controls.RELOAD);
}

bool Player::PressedFire()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyPressed(m_controls.FIRE);
	else
		return Input::ButtonPressed(m_controllerIndex, m_controls.FIRE);
}

bool Player::PressingFire()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyDown(m_controls.FIRE);
	else
		return Input::ButtonDown(m_controllerIndex, m_controls.FIRE);
}

bool Player::PressedJump()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyPressed(m_controls.JUMP);
	else
		return Input::ButtonPressed(m_controllerIndex, m_controls.JUMP);
}

bool Player::PressedCrouch()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyPressed(m_controls.CROUCH);
	else
		return Input::ButtonPressed(m_controllerIndex, m_controls.CROUCH);
}

bool Player::PressedNextWeapon()
{
	if (m_inputType == InputType::KEYBOARD_AND_MOUSE)
		return Input::KeyPressed(m_controls.NEXT_WEAPON);
	else
		return Input::ButtonPressed(m_controllerIndex, m_controls.NEXT_WEAPON);
}

void Player::SpawnBloodPool()
{		
	// Check if it's time to spawn blood pool
	if (!m_createdBloodPool && m_timeSinceDeath > 1.125)
	{
		for (RigidComponent& rigid : m_ragdoll.m_rigidComponents)
		{
			if (rigid.pxRigidBody->getName() == "RAGDOLL_HEAD")
			{
				PxTransform pose = rigid.pxRigidBody->getGlobalPose();
			//	if (pose.p.y < 0.15)
				{
					Transform transform;
					transform.position = glm::vec3(pose.p.x, 0.01f, pose.p.z);
					p_bloodPools->emplace_back(BloodPool(transform));
					m_createdBloodPool = true;
					return;
				}			
			}
		}
	}
}

void Player::SpawnMuzzleFlash()
{
	m_muzzleFlashTimer = 0;
	m_muzzleFlashRotation = Util::RandomFloat(0, HELL_PI * 2);
}

void Player::CreateCharacterController()
{
	m_characterController = PhysX::CreateCharacterController(m_transform);
}

void Player::SpawnCoprseRagdoll()
{
	p_gameCharacters->push_back(GameCharacter());

	GameCharacter* gc = &p_gameCharacters->back();
	gc->m_skinnedModel = AssetManager::GetSkinnedModelPtr("Nurse");
	gc->m_ragdoll.BuildFromJsonFile("ragdoll.json", Transform(), nullptr, PhysicsObjectType::RAGDOLL);
	gc->m_skinningMethod = SkinningMethod::RAGDOLL;
	gc->m_materialIndex = m_materialIndex;

	m_corpse = gc;

	for (int i = 0; i < gc->m_ragdoll.m_rigidComponents.size(); i++)
	{
		// get player world pos/rot
		Transform transform;
		transform.position = GetPosition();
		transform.position.y -= 1;
		transform.rotation.y = GetRotation().y + HELL_PI;

		// now match the new ragdoll pieces to the animation
		for (RigidComponent& rigidComponent : gc->m_ragdoll.m_rigidComponents)
		{
			int index = m_skinnedModel->m_BoneMapping[rigidComponent.correspondingJointName];
			PxMat44 matrix = Util::GlmMat4ToPxMat44(transform.to_mat4() * m_character_model.m_animatedTransforms.worldspace[index]);
			rigidComponent.pxRigidBody->setGlobalPose(PxTransform(matrix));
			rigidComponent.pxRigidBody->userData = new EntityData(PhysicsObjectType::RAGDOLL, &gc);

			PxShape* shape;
			rigidComponent.pxRigidBody->getShapes(&shape, 1);
			PxFilterData filterData;
			filterData.word1 = PhysX::CollisionGroup::MISC_OBSTACLE;
			filterData.word2 = PhysX::CollisionGroup::MISC_OBSTACLE;
			shape->setQueryFilterData(filterData);
			shape->setSimulationFilterData(filterData);

			rigidComponent.pxRigidBody->wakeUp();
		}
	}
}

void Player::UpdatePlayerModelAnimation(float deltaTime)
{
	// have they just switched from standing to crouch or vise versa and anim needs switching
	if (IsCrouched())
	{
		if (m_characterModelAnimationState != CharacterModelAnimationState::STOPPED_CROUCHED &&
			m_characterModelAnimationState != CharacterModelAnimationState::WALKING_CROUCHED)
		{
			if (IsMoving()) {
				m_character_model.PlayAndLoopAnimation("NurseGuyPistolCrouch.fbx");
				m_character_model.SetAnimationTime(0.1f);
				m_characterModelAnimationState = CharacterModelAnimationState::WALKING_CROUCHED;
			}

			if (!IsMoving()) {
				m_character_model.PlayAndLoopAnimation("NurseGuyPistolCrouch.fbx");
				m_characterModelAnimationState = CharacterModelAnimationState::STOPPED_CROUCHED;
			}
		}
	}	
	if (!IsCrouched())
	{
		if (m_characterModelAnimationState != CharacterModelAnimationState::WALKING &&
			m_characterModelAnimationState != CharacterModelAnimationState::STOPPED)
		{
			if (IsMoving()) {
				m_character_model.PlayAndLoopAnimation("NurseGuyPistolWalk.fbx");
				m_character_model.SetAnimationTime(0.1f);
				m_characterModelAnimationState = CharacterModelAnimationState::WALKING;
			}

			if (!IsMoving()) {
				m_character_model.PlayAndLoopAnimation("NurseGuyPistolIdle.fbx");
				m_characterModelAnimationState = CharacterModelAnimationState::STOPPED;
			}
		}
	}

	// Regular switching between anim state code BELOW

	// Standing
	if (!IsCrouched()) {
		if (IsMoving() && m_characterModelAnimationState == CharacterModelAnimationState::STOPPED) {

			m_character_model.PlayAndLoopAnimation("NurseGuyPistolWalk.fbx");
			m_character_model.SetAnimationTime(0.1f);
			m_characterModelAnimationState = CharacterModelAnimationState::WALKING;
		}

		if (!IsMoving() && m_characterModelAnimationState == CharacterModelAnimationState::WALKING)
		{
			m_character_model.PlayAndLoopAnimation("NurseGuyPistolIdle.fbx");
			m_characterModelAnimationState = CharacterModelAnimationState::STOPPED;
		}
	}
	// crouched
	else if (IsCrouched()) {
		if (IsMoving() && m_characterModelAnimationState == CharacterModelAnimationState::STOPPED_CROUCHED) {

			m_character_model.PlayAndLoopAnimation("NurseGuyPistolCrouch.fbx");
			m_character_model.SetAnimationTime(0.1f);
			m_characterModelAnimationState = CharacterModelAnimationState::WALKING_CROUCHED;
		}

		if (!IsMoving() && m_characterModelAnimationState == CharacterModelAnimationState::WALKING_CROUCHED)
		{
			m_character_model.PlayAndLoopAnimation("NurseGuyPistolCrouch.fbx");
			m_characterModelAnimationState = CharacterModelAnimationState::STOPPED_CROUCHED;
		}
	}


	if (m_isAlive)
	{
		// update animation but DON'T skin
		m_character_model.UpdateAnmation(deltaTime, false);

		Animation* animation = m_character_model.GetCurrentAnimation();
		SkinnedModel* skinnedModel = m_character_model.GetSkinnedModelPtr();

		// Get the animation time
		float AnimationTime = 0;
		if (skinnedModel->m_animations.size() > 0) {
			float TicksPerSecond = animation->m_ticksPerSecond != 0 ? animation->m_ticksPerSecond : 25.0f;
			float TimeInTicks = m_character_model.GetCurrentAnimationTime() * TicksPerSecond;
			AnimationTime = fmod(TimeInTicks, animation->m_duration);
		}



		// Traverse the tree 
		for (int i = 0; i < m_character_model.GetSkinnedModelPtr()->m_joints.size(); i++)
		{
			// Get the node and its um bind pose transform?
			const char* NodeName = skinnedModel->m_joints[i].m_name;
			glm::mat4 NodeTransformation = skinnedModel->m_joints[i].m_inverseBindTransform;

			// Calculate any animation
			if (m_character_model.GetSkinnedModelPtr()->m_animations.size() > 0)
			{
				const AnimatedNode* animatedNode = skinnedModel->FindAnimatedNode(animation, NodeName);

				if (animatedNode)
				{
					glm::vec3 Scaling;
					skinnedModel->CalcInterpolatedScaling(Scaling, AnimationTime, animatedNode);
					glm::mat4 ScalingM;

					ScalingM = Util::Mat4InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);
					glm::quat RotationQ;
					skinnedModel->CalcInterpolatedRotation(RotationQ, AnimationTime, animatedNode);
					glm::mat4 RotationM(RotationQ);

					glm::vec3 Translation;
					skinnedModel->CalcInterpolatedPosition(Translation, AnimationTime, animatedNode);
					glm::mat4 TranslationM;

					TranslationM = Util::Mat4InitTranslationTransform(Translation.x, Translation.y, Translation.z);
					NodeTransformation = TranslationM * RotationM * ScalingM;
				}
			}
			unsigned int parentIndex = skinnedModel->m_joints[i].m_parentIndex;
			//std::cout << NodeName << "\n";
			// if it is an arm, add the mouse look

			/*if (Util::StrCmp(NodeName, "mixamorig:RightArm")
				|| Util::StrCmp(NodeName, "mixamorig:LeftArm")
				|| Util::StrCmp(NodeName, "mixamorig:Head"))
			{
				Transform trans;
				trans.rotation.x = -m_camera.m_transform.rotation.x;

				NodeTransformation = NodeTransformation * trans.to_mat4();

				std::cout << "yehn\n";
			}*/

			glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : skinnedModel->m_joints[parentIndex].m_currentFinalTransform;
			glm::mat4 GlobalTransformation = ParentTransformation * NodeTransformation;

			// Store the current transformation, so child nodes can access it
			skinnedModel->m_joints[i].m_currentFinalTransform = GlobalTransformation;

			
			if (skinnedModel->m_BoneMapping.find(NodeName) != skinnedModel->m_BoneMapping.end()) {
				unsigned int BoneIndex = skinnedModel->m_BoneMapping[NodeName];
				skinnedModel->m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * skinnedModel->m_BoneInfo[BoneIndex].BoneOffset;
				skinnedModel->m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;
			}
		}

		m_character_model.m_animatedTransforms.Resize(skinnedModel->m_NumBones);

		for (unsigned int i = 0; i < skinnedModel->m_NumBones; i++) {
			m_character_model.m_animatedTransforms.local[i] = skinnedModel->m_BoneInfo[i].FinalTransformation;
			m_character_model.m_animatedTransforms.worldspace[i] = skinnedModel->m_BoneInfo[i].ModelSpace_AnimatedTransform;
			// Transforms[i] = m_BoneInfo[i].FinalTransformation;
			// DebugAnimatedTransforms[i] = m_BoneInfo[i].ModelSpace_AnimatedTransform;
		}
	}
	else
	{
		m_skinnedModel->UpdateBoneTransformsFromRagdoll(m_character_model.m_animatedTransforms.local, m_character_model.m_animatedTransforms.worldspace, &m_ragdoll);
	}

}



void Player::CreatePoormansCharacterController()
{
	return;

	// Sphere size
	float radius = 0.25f;

	// Spawn location
	Transform spawnTransform = m_transform;
	spawnTransform.position += radius + 0.05;
	PxMat44 spawnMatrix = Util::GlmMat4ToPxMat44(m_transform.to_mat4());

	// Create rigid
	m_rigid = PhysX::GetPhysics()->createRigidDynamic(PxTransform(spawnMatrix));
	m_rigid->setSolverIterationCounts(8, 1);
	m_rigid->setName("PLAYER_SPHERE");
	m_rigid->userData = new EntityData(PhysicsObjectType::UNDEFINED, this);


	// Create and attach shape
	auto* material = PhysX::GetDefaultMaterial();
	PxSphereGeometry geometry = PxSphereGeometry(radius);
	PxShape* shape = PxRigidActorExt::createExclusiveShape(*m_rigid, geometry, *material);

	// Add to scene
	PxScene* scene = PhysX::GetScene();
	scene->addActor(*m_rigid);
	float mass = 1;
	PxRigidBodyExt::setMassAndUpdateInertia(*m_rigid, mass);
}

int Player::GetCurrentGunAmmoInClip()
{
	if (m_gun == Gun::GLOCK)
		return m_ammo_glock_in_clip;
	if (m_gun == Gun::SHOTGUN)
		return m_ammo_shotgun_in_clip;
	return -1;
}

int Player::GetCurrentGunTotalAmmo()
{
	if (m_gun == Gun::GLOCK)
		return m_ammo_glock_total;
	if (m_gun == Gun::SHOTGUN)
		return m_ammo_shotgun_total;
	return -1;
}

void Player::FireBullet(float variance, float bulletForce)
{
	bool bodyHit = false;
	bool headHit = false;

	FireBulletRay(variance);

	if (m_bulletRay.HitFound())
	{
		// bullet decals
		if (m_bulletRay.m_hitObjectName == "WALL" || m_bulletRay.m_hitObjectName == "GROUND" )
		{
			GameData::s_bulletDecals.push_back(BulletDecal());
			BulletDecal* decal = &GameData::s_bulletDecals.back();
			decal->m_position = m_bulletRay.m_hitPosition;
			decal->m_normal = m_bulletRay.m_surfaceNormal;
		}


		if (m_bulletRay.m_hitObjectName == "RAGDOLL"
			|| m_bulletRay.m_hitObjectName == "RAGDOLL_HEAD")
		{
			PxVec3 force = PxVec3(m_camera.m_Front.x, m_camera.m_Front.y, m_camera.m_Front.z) * bulletForce;

			PxRigidDynamic* actor = (PxRigidDynamic*)m_bulletRay.m_hitActor;
			actor->addForce(force);

			EntityData* physicsData = (EntityData*)actor->userData;

			// blood noise
			if (physicsData->type == PhysicsObjectType::RAGDOLL || physicsData->type == PhysicsObjectType::PLAYER_RAGDOLL)
			{
				bodyHit = true;
			}

			// kill em if it's player
			if (physicsData->type == PhysicsObjectType::PLAYER_RAGDOLL) {

				Player* p = (Player*)physicsData->parent;

				// kill em
				if (m_bulletRay.m_hitObjectName == "RAGDOLL_HEAD" && p->m_isAlive) {
					p->m_isAlive = false;
					m_killCount++;
					headHit = true;

					// first disable players ragdoll from raycasts
					for (RigidComponent& rigid : p->m_ragdoll.m_rigidComponents) {
						PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
						PhysX::DisableRayCastingForShape(shape);
					}

					// CREATE A RAGDOLL
					p->SpawnCoprseRagdoll();

					// enable corpse for raycasts
					for (RigidComponent& rigid : p->m_corpse->m_ragdoll.m_rigidComponents) {
						PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
						PhysX::EnableRayCastingForShape(shape);
					}

					PhysX::GetScene()->simulate(std::min(0.01f, 0.01f));
					PhysX::GetScene()->fetchResults(true);

					FireBulletRay(m_bulletRay.m_rayDirection); // hit the new ragdoll
					if (m_bulletRay.HitFound())
					{
						std::cout << "hit found, was:\n";
						std::cout << m_bulletRay.m_hitObjectName << "\n";
						if (m_bulletRay.m_hitObjectName == "RAGDOLL" || m_bulletRay.m_hitObjectName == "RAGDOLL_HEAD")
						{
							//	std::cout << "was ragdoll\n";
							PxVec3 force = PxVec3(m_camera.m_Front.x, m_camera.m_Front.y, m_camera.m_Front.z) * bulletForce;

							PxRigidDynamic* actor = (PxRigidDynamic*)m_bulletRay.m_hitActor;
							actor->addForce(force);
						}
					}
				}
			}
		}
	}

	if (headHit) {
		std::string file = "Death0.wav";
		Audio::PlayAudio(file.c_str(), 0.45f);
	}
	else if (bodyHit) {
			std::string file = "FLY_Bullet_Impact_Flesh_0" + std::to_string(rand() % 8 + 1) + ".wav";
			Audio::PlayAudio(file.c_str(), 0.5f);
	}	
}

void Player::CheckForWeaponInput()
{

	// a better way to do this is to set it to true first and THEN check for the stuff that disallows it

	bool canChangeWeapon = false;

	if (m_gun == Gun::SHOTGUN)
		canChangeWeapon = true;
	else if (m_gun == Gun::GLOCK && m_HUDWeaponAnimationState == HUDWeaponAnimationState::RELOADING && m_HUD_Weapon.AnimationIsPastPercentage(50))
		canChangeWeapon = true;
	else
		canChangeWeapon = true;



	// lazy weapon switch implementation
	if (canChangeWeapon)
	{
		/*if (m_HUDWeaponAnimationState == HUDWeaponAnimationState::IDLE ||
			m_HUDWeaponAnimationState == HUDWeaponAnimationState::RELOADING ||
			m_HUDWeaponAnimationState == HUDWeaponAnimationState::FIRING
			)
		{*/

			if (PressedNextWeapon() && m_gun == Gun::SHOTGUN) {
				//if (Input::KeyPressed(HELL_KEY_1)) {
				m_gunToChangeTo = Gun::GLOCK;
				Audio::PlayAudio("Glock_Equip.wav", 0.5f);
				m_HUD_Weapon.PlayAnimation("Shotgun_Dequip.fbx");
				m_HUDWeaponAnimationState = HUDWeaponAnimationState::HOLSTERING;
			}
			else if (PressedNextWeapon() && m_gun == Gun::GLOCK) {
				//else if (Input::KeyPressed(HELL_KEY_2)) {
				m_gunToChangeTo = Gun::SHOTGUN;
				Audio::PlayAudio("Glock_Equip.wav", 0.5f);
				m_HUD_Weapon.PlayAnimation("Glock_Holster.fbx");
				m_HUDWeaponAnimationState = HUDWeaponAnimationState::HOLSTERING;
			}
		//}
	}

	
	// Finished holstering
	if (m_HUD_Weapon.AnimationIsComplete() && m_HUDWeaponAnimationState == HUDWeaponAnimationState::HOLSTERING)			
	{
		if (m_gunToChangeTo == Gun::GLOCK) {
			m_gun = Gun::GLOCK;
			m_HUDWeaponAnimationState = HUDWeaponAnimationState::EQUIPPING;
			m_HUD_Weapon.SetSkinnedModel(AssetManager::GetSkinnedModelPtr("Glock"));
			m_HUD_Weapon.PlayAnimation("Glock_Equip2.fbx", 1.25f);
			m_gunToChangeTo = Gun::NONE;
		}		
		
		else if (m_gunToChangeTo == Gun::SHOTGUN) {
			m_gun = Gun::SHOTGUN;
			m_HUDWeaponAnimationState = HUDWeaponAnimationState::EQUIPPING;
			m_HUD_Weapon.SetSkinnedModel(AssetManager::GetSkinnedModelPtr("Shotgun"));
			m_HUD_Weapon.PlayAnimation("Shotgun_Equip.fbx", 1.25f);
			m_gunToChangeTo = Gun::NONE;
		}
	}		


	if (m_gun == Gun::SHOTGUN) {

		// finished whatever you were doing? (at least i think this is what it does, read the glock comment below)
		if (m_HUD_Weapon.AnimationIsComplete())
		{
			if (m_HUDWeaponAnimationState == HUDWeaponAnimationState::FIRING ||
				m_HUDWeaponAnimationState == HUDWeaponAnimationState::EQUIPPING)
			{
				m_HUD_Weapon.PlayAndLoopAnimation("Shotgun_Idle.fbx");
				m_HUDWeaponAnimationState = HUDWeaponAnimationState::IDLE;
			}
		}

		// Fire shotgun
		if (PressedFire())
		{
			if (GetCurrentGunAmmoInClip() > 0) 
			{
				// first check you aren't in the first 50% of a fire animation
				bool stillFiring = false;

				if (m_HUDWeaponAnimationState == HUDWeaponAnimationState::FIRING && m_HUD_Weapon.GetCurrentAnimationTime() < 0.5)
					stillFiring = true;

				if (!stillFiring)
				{
					m_HUDWeaponAnimationState = HUDWeaponAnimationState::FIRING;
					m_HUD_Weapon.PlayAnimation("Shotgun_Fire.fbx", 1.25f);
					FireShotgun();
					SpawnMuzzleFlash();
				}
			}
		}

		// Press reload
		if (PressedReload() && m_ammo_shotgun_total > 0)
		{
			if (m_HUDWeaponAnimationState != HUDWeaponAnimationState::RELOADING &&
				m_HUDWeaponAnimationState != HUDWeaponAnimationState::EQUIPPING &&
				m_HUDWeaponAnimationState != HUDWeaponAnimationState::HOLSTERING)
			{
				m_HUDWeaponAnimationState = HUDWeaponAnimationState::RELOADING;
				m_shotgunReloadState = ShotgunReloadState::FROM_IDLE;
				m_HUD_Weapon.PlayAnimation("Shotgun_ReloadWetStart.fbx", 1.5f);
			}
		}

		// Animated reload
		if (m_HUDWeaponAnimationState == HUDWeaponAnimationState::RELOADING)
		{
			//  there is still reloading to do?... 
			if (m_ammo_shotgun_in_clip < m_clip_size_shotgun) 
			{
				// done flipping it? then load shells
				if (m_HUD_Weapon.AnimationIsComplete() && m_shotgunReloadState == ShotgunReloadState::FROM_IDLE ||
					m_HUD_Weapon.AnimationIsComplete() && m_shotgunReloadState == ShotgunReloadState::DOUBLE_RELOAD ||
					m_HUD_Weapon.AnimationIsComplete() && m_shotgunReloadState == ShotgunReloadState::SINGLE_RELOAD)
				{
					// 2 shell
					if (m_ammo_shotgun_in_clip <= 6 && m_ammo_shotgun_total > 2)
					{
						m_shotgunReloadState = ShotgunReloadState::DOUBLE_RELOAD;
						m_HUD_Weapon.PlayAnimation("Shotgun_Reload2Shells.fbx", 1.5f);
						//m_ammo_shotgun_in_clip += 2;					
						m_firstShellLoaded = false;
						m_secondShellLoaded = false;
					}
					// 1 shell
					else if (m_ammo_shotgun_total > 0)
					{
						m_shotgunReloadState = ShotgunReloadState::SINGLE_RELOAD;
						m_HUD_Weapon.PlayAnimation("Shotgun_Reload1Shell.fbx", 1.5f);
						//m_ammo_shotgun_in_clip++;						
						m_firstShellLoaded = false;
						m_secondShellLoaded = false;
					}
				}
			}
			// pack it up
			if (m_HUD_Weapon.AnimationIsComplete() && m_shotgunReloadState != ShotgunReloadState::BACK_TO_IDLE)
			{
				if (m_ammo_shotgun_in_clip == m_clip_size_shotgun || m_ammo_shotgun_total == 0) {
					m_shotgunReloadState = ShotgunReloadState::BACK_TO_IDLE;
					m_HUD_Weapon.PlayAnimation("Shotgun_ReloadEnd.fbx", 1.5f);
					Audio::PlayAudio("Shotgun_ReloadEnd.wav");
				}
			}

			// sounds and counter increment
			if (m_shotgunReloadState == ShotgunReloadState::DOUBLE_RELOAD)
			{
				if (!m_firstShellLoaded && m_HUD_Weapon.GetCurrentAnimationTime() > 0.56f) {
					m_firstShellLoaded = true;
					Audio::PlayAudio("Shotgun_Reload_01.wav");
					m_ammo_shotgun_in_clip++;
					m_ammo_shotgun_total--;
				}
				if (!m_secondShellLoaded && m_HUD_Weapon.GetCurrentAnimationTime() > 1.03f) {
					m_secondShellLoaded = true;
					Audio::PlayAudio("Shotgun_Reload_05.wav");
					m_ammo_shotgun_in_clip++;
					m_ammo_shotgun_total--;
				}
			}
			// sounds and counter increment
			if (m_shotgunReloadState == ShotgunReloadState::SINGLE_RELOAD)
			{
				if (!m_firstShellLoaded && m_HUD_Weapon.GetCurrentAnimationTime() > 0.45f) {
					m_firstShellLoaded = true;
					Audio::PlayAudio("Shotgun_Reload_01.wav");
					m_ammo_shotgun_in_clip++;
					m_ammo_shotgun_total--;
				}
			}


			// are you finished rolling back to idle?
			if (m_HUD_Weapon.AnimationIsComplete() && m_shotgunReloadState == ShotgunReloadState::BACK_TO_IDLE)
			{
				m_shotgunReloadState = ShotgunReloadState::NOT_RELOADING;
				m_HUD_Weapon.PlayAndLoopAnimation("Shotgun_Idle.fbx");
				m_HUDWeaponAnimationState = HUDWeaponAnimationState::IDLE;
			}
		}

		// Animate idle
		if (!IsMoving() && m_HUDWeaponAnimationState == HUDWeaponAnimationState::IDLE) {
			m_HUD_Weapon.PlayAndLoopAnimation("Shotgun_Idle.fbx");
		}
		// Animate walking
		if (IsMoving() && m_HUDWeaponAnimationState == HUDWeaponAnimationState::IDLE) {
			m_HUD_Weapon.PlayAndLoopAnimation("Shotgun_Walk.fbx");
		}

		// THIS MASKS A BUG SURELY - look into it at some point
		if (m_HUDWeaponAnimationState == HUDWeaponAnimationState::EQUIPPING && m_HUD_Weapon.AnimationIsPastPercentage(95))
		{
			m_HUD_Weapon.PlayAndLoopAnimation("Shotgun_Idle.fbx");
			m_HUDWeaponAnimationState = HUDWeaponAnimationState::IDLE;
		}
	}


	if (m_gun == Gun::GLOCK) {
		// FIRST, check and reset states, hopefully this fixes that weird reload bug but keep an eye out for it: reload anim doesn't play, but sound does and ammo is credited.


		if (m_HUD_Weapon.AnimationIsComplete())
		{
			if (m_HUDWeaponAnimationState == HUDWeaponAnimationState::FIRING ||
				m_HUDWeaponAnimationState == HUDWeaponAnimationState::RELOADING ||
				m_HUDWeaponAnimationState == HUDWeaponAnimationState::EQUIPPING)
			{
				m_HUD_Weapon.PlayAndLoopAnimation("Glock_Idle.fbx");
				m_HUDWeaponAnimationState = HUDWeaponAnimationState::IDLE;
			}
		}

		// Presses fire
		if (PressedFire())
		{		
			// and has ammo
			if (GetCurrentGunAmmoInClip() > 0)
			{
				// fire glock if idle (aka not reloading, or reloading animation is 75 percent complete)
				if (m_HUDWeaponAnimationState != HUDWeaponAnimationState::RELOADING ||
					m_HUDWeaponAnimationState == HUDWeaponAnimationState::RELOADING && m_HUD_Weapon.AnimationIsPastPercentage(75))
				{
					m_HUDWeaponAnimationState = HUDWeaponAnimationState::FIRING;
					m_HUD_Weapon.PlayAnimation("Glock_Fire" + std::to_string(rand() % 3) + ".fbx");
					FireGlock();
					SpawnGlockCasing();
					SpawnMuzzleFlash();
				}
			}
			// is empty
			else {
				std::string file = "Empty.wav";
				Audio::PlayAudio(file.c_str(), 0.5f);
			}
			
		}

		// Presses Reload
		if (PressedReload()
			&& m_HUDWeaponAnimationState != HUDWeaponAnimationState::RELOADING
			&& m_ammo_glock_in_clip != m_clip_size_glock
			&& m_ammo_glock_total > 0)
		{
			m_HUD_Weapon.PlayAnimation("Glock_Reload.fbx");
			m_HUDWeaponAnimationState = HUDWeaponAnimationState::RELOADING;
			Audio::PlayAudio("Glock_Reload.wav", 0.5f);

			unsigned int ammo_to_add = std::min(m_clip_size_glock - m_ammo_glock_in_clip, m_ammo_glock_total);
			m_ammo_glock_in_clip += ammo_to_add;
			m_ammo_glock_total -= ammo_to_add;
		}

		// idle
		if (!IsMoving() && m_HUDWeaponAnimationState == HUDWeaponAnimationState::IDLE) {
			m_HUD_Weapon.PlayAndLoopAnimation("Glock_Idle.fbx");
			//m_HUD_Weapon.PlayAndLoopAnimation("Glock_Walk.fbx");
		}
		// walking
		if (IsMoving() && m_HUDWeaponAnimationState == HUDWeaponAnimationState::IDLE) {
			m_HUD_Weapon.PlayAndLoopAnimation("Glock_Walk.fbx");
		}

		// THIS MASKS A BUG SURELY - look into it at some point
		if (m_HUDWeaponAnimationState == HUDWeaponAnimationState::EQUIPPING && m_HUD_Weapon.AnimationIsPastPercentage(95))
		{
		//	m_HUD_Weapon.PlayAndLoopAnimation("Glock_Idle.fbx");
		//	m_HUDWeaponAnimationState = HUDWeaponAnimationState::IDLE;
		}
	}



	// Presses Equip
	if (Input::KeyPressed(HELL_KEY_J) && m_gun == Gun::GLOCK)
	{
		m_HUD_Weapon.SetSkinnedModel(AssetManager::GetSkinnedModelPtr("Glock"));
		m_HUD_Weapon.PlayAnimation("Glock_FirstEquip1.fbx");
		m_HUDWeaponAnimationState = HUDWeaponAnimationState::EQUIPPING;
		Audio::PlayAudio("Glock_Equip.wav", 0.5f);
	}

	// Were you just born????
	if (m_justBorn) 
	{
		m_HUD_Weapon.SetSkinnedModel(AssetManager::GetSkinnedModelPtr("Glock"));
		m_HUD_Weapon.PlayAnimation("Glock_FirstEquip1.fbx");
		m_HUDWeaponAnimationState = HUDWeaponAnimationState::EQUIPPING;
		Audio::PlayAudio("Glock_Equip.wav", 0.5f);

		m_gun = Gun::GLOCK;
		m_justBorn = false;
		std::cout << "BORN\n";
	}

}

/*template <class T>
bool IsPlayer(T* t) { return false; } // normal case returns false 

template <>
bool IsPlayer(Player* t) { return true; }*/


void Player::FireGlock()
{
	m_ammo_glock_in_clip--;
	FireBullet(0, 1500);

	std::string file = "Glock_Fire_" + std::to_string(rand() % 3) + ".wav";
	Audio::PlayAudio(file.c_str(), 0.5f);
}

void Player::FireShotgun()
{
	m_ammo_shotgun_in_clip--;
	
	for (int i=0; i<12;i++)
		FireBullet(0.125f, 1500);

	Audio::PlayAudio("Shotgun_Fire_01.wav", 0.75f);
}

void Player::SpawnGlockCasing()
{
	// Get world pos of barrel
	SkinnedModel* glock = AssetManager::GetSkinnedModelPtr("Glock");
	unsigned int BoneIndex = glock->m_BoneMapping["barrel"];
	glm::mat4 BoneMatrix = glm::mat4(1);	
	if (m_HUD_Weapon.m_animatedTransforms.worldspace.size())
		BoneMatrix = m_HUD_Weapon.m_animatedTransforms.worldspace[BoneIndex];
	



	static float x = 0;
	static float y = 0;
	static float z = 0;
	static float rx = 0;
	static float ry = 0;
	static float rz = 0;

	/*if (Input::s_keyPressed[HELL_KEY_L]) {
		x = 0;
		y = 0;
		z = 0;
		rx = 0;
		ry = 0;
		rz = 0;
	}
	if (Input::s_keyPressed[HELL_KEY_9])
		rx += 0.1f;
	if (Input::s_keyPressed[HELL_KEY_0])
		ry += 0.1f;
	if (Input::s_keyPressed[HELL_KEY_MINUS])
		rz += 0.1f;
	if (Input::s_keyPressed[HELL_KEY_I])
		rx -= 0.1f;
	if (Input::s_keyPressed[HELL_KEY_O])
		ry -= 0.1f;
	if (Input::s_keyPressed[HELL_KEY_P])
		rz -= 0.1f;

	if (Input::s_keyPressed[HELL_KEY_3])
		x += 0.1f;
	if (Input::s_keyPressed[HELL_KEY_4])
		y += 0.1f;
	if (Input::s_keyPressed[HELL_KEY_5])
		z += 0.1f;
	if (Input::s_keyPressed[HELL_KEY_6])
		x -= 0.1f;
	if (Input::s_keyPressed[HELL_KEY_7])
		y -= 0.1f;
	if (Input::s_keyPressed[HELL_KEY_8])
		z -= 0.1f;

	//std::cout << "a: " << x << ", " << y << ", " << z << "\n";
	//std::cout << "b: " << rx << ", " << ry << ", " << rz << "\n";
	*/


	Transform offsetTransform;
	//glm::vec3 forward = glm::normalize(glm::vec3(m_camera.m_Front.x, 0, m_camera.m_Front.z));
	offsetTransform.position = m_camera.m_Front * glm::vec3(0.5);
	offsetTransform.position += m_camera.m_Up * glm::vec3(rx, ry, rz);
	offsetTransform.position += m_camera.m_Right * glm::vec3(x, y, z);

	glm::mat4 worldMatrix = offsetTransform.to_mat4() * m_HUD_Weapon.GetTransform().to_mat4() * BoneMatrix;
	glm::vec3 spawnPosition = Util::TranslationFromMat4(worldMatrix);

	m_debugPos = spawnPosition;

	return;

	/*Transform t;
	//t.position = GetGlockCasingSpawnWorldPosition();
	t.position = GetGlockBarrelHoleWorldPosition();// it look sbetter spawning at the barrel lol. too close otherwise
	t.rotation = p_camera->m_transform.rotation;*/

	glm::vec3 initialVelocity;
	initialVelocity = glm::normalize(m_camera.m_Up + (m_camera.m_Right * glm::vec3(3.0f)));
	initialVelocity *= glm::vec3(1.5f);

	Transform trans;
	trans.position = spawnPosition;
	trans.rotation = m_camera.m_transform.rotation;

	PxRigidDynamic* rigid = PhysX::CreateBox(trans, initialVelocity);
	rigid->putToSleep();

	//Shell::s_bulletCasings.push_back(Shell(t, initialVelocity, CasingType::BULLET_CASING));*/
}

void Player::SetControlsToDefaultPS4Controls()
{
	m_controls.WALK_FORWARD =	HELL_PS_4_CONTROLLER_DPAD_UP;
	m_controls.WALK_BACKWARD =	HELL_PS_4_CONTROLLER_DPAD_DOWN;
	m_controls.WALK_LEFT =		HELL_PS_4_CONTROLLER_DPAD_LEFT;
	m_controls.WALK_RIGHT =		HELL_PS_4_CONTROLLER_DPAD_RIGHT;
	m_controls.INTERACT =		HELL_PS_4_CONTROLLER_TRIANGLE;
	m_controls.RELOAD =			HELL_PS_4_CONTROLLER_SQUARE;
	m_controls.FIRE =			HELL_PS_4_CONTROLLER_TRIGGER_R;
	m_controls.JUMP =			HELL_PS_4_CONTROLLER_CROSS;
	m_controls.CROUCH =			HELL_PS_4_CONTROLLER_TRIGGER_L;
	m_controls.NEXT_WEAPON =	HELL_PS_4_CONTROLLER_CROSS;
}

int Player::GetControllerIndex()
{
	return m_controllerIndex;
}

void Player::SetControllerIndex(int index)
{
	// Store the index
	m_controllerIndex = index;

	// Change input type
	m_inputType = InputType::CONTROLLER;

	// Retrieve the button count
	int buttonCount;
	const unsigned char* buttons = glfwGetJoystickButtons(index, &buttonCount);
	
	// Resize the buttons vector, 22 should be enough, final 2 buttons are reserved for triggers
	Input::s_controllerStates[index].buttons_down.resize(22);
	Input::s_controllerStates[index].buttons_down_last_frame.resize(22);
	Input::s_controllerStates[index].buttons_pressed.resize(22);

	// Wipe the button states to false
	std::fill(Input::s_controllerStates[index].buttons_down.begin(), Input::s_controllerStates[index].buttons_down.end(), false);
	std::fill(Input::s_controllerStates[index].buttons_down_last_frame.begin(), Input::s_controllerStates[index].buttons_down_last_frame.end(), false);
	std::fill(Input::s_controllerStates[index].buttons_pressed.begin(), Input::s_controllerStates[index].buttons_pressed.end(), false);

	// Set to default PS4 controls. YOU WILL DEFINITELY HAVE TO CHANGE THIS LATER!!!
	SetControlsToDefaultPS4Controls();
}


void Player::ForceRagdollToMatchAnimation()
{
//	return;
	Transform transform;
	transform.position = GetPosition();
	transform.position.y -= 1;
	transform.rotation.y = GetRotation().y + HELL_PI;

	for (RigidComponent& rigidComponent : m_ragdoll.m_rigidComponents)
	{
		int index = m_skinnedModel->m_BoneMapping[rigidComponent.correspondingJointName];
		PxMat44 matrix = Util::GlmMat4ToPxMat44(transform.to_mat4() * m_character_model.m_animatedTransforms.worldspace[index]);
		rigidComponent.pxRigidBody->setGlobalPose(PxTransform(matrix));
		rigidComponent.pxRigidBody->putToSleep();
	}
}

void Player::SetPosition(glm::vec3 position)
{
	m_transform.position = position;
}

glm::vec3 Player::GetPosition()
{
	return m_transform.position;
}

glm::vec3 Player::GetRotation()
{
	return m_camera.m_transform.rotation;
}

glm::mat4& Player::GetCameraViewMatrix()
{	
	if (m_isAlive) 
	{
		const PxExtendedVec3& globalPose = m_characterController->getPosition();
		m_transform.position = glm::vec3(globalPose.x, globalPose.y - 0.3f, globalPose.z);

		// camera position (it's player plus view height)
		m_camera.m_transform.position = m_transform.position;
		m_camera.m_transform.position.y += m_cameraViewHeight;



		// weapon transform 
		Transform weaponTransform;
		weaponTransform.position = m_camera.m_transform.position;
		weaponTransform.rotation = m_camera.m_transform.rotation;
		weaponTransform.scale = glm::vec3(0.001);

		// Add sway
		//weaponTransform.position += m_camera.m_swayPosition * glm::vec3(0.001);

		m_HUD_Weapon.SetTransform(weaponTransform);


		// glock node 102 is Camera001 //
		glm::mat4 animatedCameraMatrix = m_HUD_Weapon.m_animatedTransforms.cameraMatrix;


		// update matrices and return bish
		m_camera.CalculateMatrices(animatedCameraMatrix);


		return m_camera.m_viewMatrix;
	}
	else
	{
		for (RigidComponent& rigid : m_corpse->m_ragdoll.m_rigidComponents) 
		{
			if (rigid.correspondingJointName == "mixamorig:Neck")
			{
				/*static float rx = 0;
				static float ry = 0;
				static float rz = 0;

				if (Input::s_keyPressed[HELL_KEY_L]) {
					rx = 0;
					ry = 0;
					rz = 0;
				}

				if (Input::s_keyPressed[HELL_KEY_9])
					rx += 0.1f;
				if (Input::s_keyPressed[HELL_KEY_0])
					ry += 0.1f;
				if (Input::s_keyPressed[HELL_KEY_MINUS])
					rz += 0.1f;
				if (Input::s_keyPressed[HELL_KEY_I])
					rx -= 0.1f;
				if (Input::s_keyPressed[HELL_KEY_O])
					ry -= 0.1f;
				if (Input::s_keyPressed[HELL_KEY_P])
					rz -= 0.1f;

				std::cout << "rot: " << rx << ", " << ry << ", " << rz << "\n";*/

				Transform offsetTransform;
				offsetTransform.rotation = glm::vec3(0.3, 3.3, -0.2);

				


				PxTransform transform = rigid.pxRigidBody->getGlobalPose();

				m_camera.m_viewMatrix = glm::inverse(Util::PxMat44ToGlmMat4(PxMat44(transform)) * offsetTransform.to_mat4());

				m_camera.m_inverseViewMatrix = glm::inverse(m_camera.m_viewMatrix);
				m_camera.m_Right = glm::vec3(m_camera.m_inverseViewMatrix[0]);//*glm::vec3(-1, -1, -1);
				m_camera.m_Up = glm::vec3(m_camera.m_inverseViewMatrix[1]);// *glm::vec3(-1, -1, -1);
				m_camera.m_Front = glm::vec3(m_camera.m_inverseViewMatrix[2]) * glm::vec3(-1, -1, -1);

				glm::vec4 vP = (m_camera.m_inverseViewMatrix * glm::vec4(0, 0, 0, 1));
				m_camera.m_viewPos = glm::vec3(vP.x, vP.y, vP.z);

				m_camera.m_projectionViewMatrix = m_camera.m_projectionMatrix * m_camera.m_viewMatrix;

				m_debugMat = m_camera.m_viewMatrix;

				return m_camera.m_viewMatrix;
			}
		}
	}



}

glm::vec3 Player::GetViewPos()
{
	return GetPosition() + glm::vec3(0, m_cameraViewHeight, 0);
}

Camera* Player::GetCameraPointer()
{
	return &m_camera;
}

bool Player::IsMoving()
{
	return m_isMoving;
}

bool Player::IsCrouched()
{
	return m_isCrouching;
}

void Player::RenderCharacterModel(Shader* shader)
{
	if (!m_skinnedModel) {
		std::cout << "CHARACTER MODEL NOT SET. BAILING ON TRYING TO RENDER IT.\n";
		return;
	}

	Transform transform;
	transform.position = GetPosition();
	transform.position.y -= 1;
	transform.rotation.y = GetRotation().y + HELL_PI;
	glm::mat4 modelMatrix = transform.to_mat4();

	if (!m_isAlive)
		modelMatrix = glm::mat4(1);

	shader->setInt("hasAnimation", true);

	for (unsigned int i = 0; i < m_character_model.m_animatedTransforms.local.size(); i++)
		shader->setMat4("skinningMats[" + std::to_string(i) + "]", modelMatrix * m_character_model.m_animatedTransforms.local[i]);

	glBindVertexArray(m_skinnedModel->m_VAO);
	glActiveTexture(GL_TEXTURE0);

	for (int i = 0; i < m_skinnedModel->m_meshEntries.size(); i++) 
	{
		if (m_materialIndex == 0)
			if (m_skinnedModel->m_meshEntries[i].material)
				m_skinnedModel->m_meshEntries[i].material->Bind();

		if (m_materialIndex == 1)
			if (m_skinnedModel->m_meshEntries[i].materialB)
				m_skinnedModel->m_meshEntries[i].materialB->Bind();

		glDrawElementsBaseVertex(GL_TRIANGLES, m_skinnedModel->m_meshEntries[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * m_skinnedModel->m_meshEntries[i].BaseIndex), m_skinnedModel->m_meshEntries[i].BaseVertex);
	}

	shader->setInt("hasAnimation", false);
}



void Player::SetCharacterModel(SkinnedModel* skinnedModel)
{
	m_skinnedModel = skinnedModel;
}

glm::mat4& Player::GetCameraProjectionMatrix()
{
	return m_camera.m_projectionMatrix;
}

void Player::HitFloor()
{
	if (!m_hasHitFloorYet)
	{
		m_hasHitFloorYet = true;

		std::string file = "BodyFall_01.wav";
		Audio::PlayAudio(file.c_str(), 0.5f);
	}
}