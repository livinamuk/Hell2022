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
	m_ammo_total = 80;
	m_ammo_in_clip = 15;
	m_isAlive = true;
	m_transform = Transform();
	m_hasHitFloorYet = false;
	m_transform.position = glm::vec3(Util::RandomFloat(-10, 10), 0, Util::RandomFloat(-10, 10));
	m_camera.m_transform.rotation = glm::vec3(0, Util::RandomFloat(0, 6), 0);
	m_justBorn = true;
	m_createdBloodPool = false;

	std::string file = "UI_Select2.wav";
	Audio::PlayAudio(file.c_str(), 0.5f);

	GameCharacter gameCharacter;
	gameCharacter.m_transform.position = glm::vec3(0, 5, 0);
	gameCharacter.m_transform.rotation.x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HELL_PI)));
	gameCharacter.m_transform.rotation.y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HELL_PI)));
	gameCharacter.m_transform.rotation.z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HELL_PI)));
	gameCharacter.m_skinnedModel = AssetManager::GetSkinnedModelPtr("Nurse");
	gameCharacter.m_ragdoll.BuildFromJsonFile("ragdoll.json", gameCharacter.m_transform, nullptr, PhysicsObjectType::RAGDOLL);
	gameCharacter.m_skinningMethod = SkinningMethod::RAGDOLL;
	gameCharacter.m_materialIndex = m_materialIndex;

	p_gameCharacters->push_back(gameCharacter);

	GameCharacter* gc = &(*p_gameCharacters)[p_gameCharacters->size() - 1];

	for (int i = 0; i < gc->m_ragdoll.m_rigidComponents.size(); i++)
	{
		PxTransform transform = m_ragdoll.m_rigidComponents[i].pxRigidBody->getGlobalPose();
		gc->m_ragdoll.m_rigidComponents[i].pxRigidBody->setGlobalPose(transform);
		gc->m_ragdoll.m_rigidComponents[i].pxRigidBody->putToSleep();
		gc->m_ragdoll.m_rigidComponents[i].pxRigidBody->userData = new EntityData(PhysicsObjectType::RAGDOLL, &gameCharacter);

	}
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

	// Keyboard / controller button movement
	if (PressingWalkForward()) {
		m_transform.position += Forward * deltaTime * speed;
		m_isMoving = true;
	}
	if (PressingWalkBackward()) {
		m_transform.position -= Forward * deltaTime * speed;
		m_isMoving = true;
	}
	if (PressingWalkLeft()) {
		m_transform.position -= m_camera.m_Right * deltaTime * speed;
		m_isMoving = true;
	}
	if (PressingWalkRight()) {
		m_transform.position += m_camera.m_Right * deltaTime * speed;
		m_isMoving = true;
	}

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

	FootstepAudio(deltaTime);
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
		float aim_trigger_axis_X;
		float aim_trigger_axis_Y;
		if (m_leftStickMode == ControllerStickMode::AIMING) {
			aim_trigger_axis_X = Input::s_controllerStates[m_controllerIndex].left_stick_axis_X;
			aim_trigger_axis_Y = Input::s_controllerStates[m_controllerIndex].left_stick_axis_Y;
		}
		if (m_rightStickMode == ControllerStickMode::AIMING) {
			aim_trigger_axis_X = Input::s_controllerStates[m_controllerIndex].right_stick_axis_X;
			aim_trigger_axis_Y = Input::s_controllerStates[m_controllerIndex].right_stick_axis_Y;
		}

		// Prevent drifting
		float deadZone = 0.1f;
		if (aim_trigger_axis_X < deadZone && aim_trigger_axis_X > -deadZone)
			aim_trigger_axis_X = 0;
		if (aim_trigger_axis_Y < deadZone && aim_trigger_axis_Y > -deadZone)
			aim_trigger_axis_Y = 0;

		// Apply sensitivity
		aim_trigger_axis_X *= 0.08;
		aim_trigger_axis_Y *= 0.08;

		// Apply actual trigger position to the camera
		float yLimit = 1.5f;
		m_camera.m_transform.rotation += glm::vec3(-aim_trigger_axis_Y, -aim_trigger_axis_X, 0.0);	
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

void Player::CreateCharacterController()
{
	m_characterController = PhysX::CreateCharacterController(m_transform);
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



void Player::CheckForWeaponInput()
{

	// FIRST, check and reset states, hopefully this fixes that weird reload bug but keep an eye out for it: reload anim doesn't play, but sound does and ammo is credited.
	if (m_HUD_Weapon.AnimationIsComplete())
	{
		if (m_HUDWeaponAnimationState == HUDWeaponAnimationState::FIRING ||
			m_HUDWeaponAnimationState == HUDWeaponAnimationState::RELOADING)
		{
			m_HUD_Weapon.PlayAndLoopAnimation("Glock_Idle.fbx");
			m_HUDWeaponAnimationState = HUDWeaponAnimationState::IDLE;
		}
	}

	if (m_justBorn)
	{
		m_HUD_Weapon.PlayAnimation("Glock_FirstEquip1.fbx");
		m_HUDWeaponAnimationState = HUDWeaponAnimationState::EQUIPPING;
		Audio::PlayAudio("Glock_Equip.wav", 0.5f);
		m_justBorn = false;
	}

	// Presses fire
	if (PressedFire())
	{
		// and has ammo
		if(m_ammo_in_clip > 0)
		{
			// fire glock if idle (aka not reloading, or reloading animation is 75 percent complete)
			if (m_HUDWeaponAnimationState != HUDWeaponAnimationState::RELOADING ||
				m_HUDWeaponAnimationState == HUDWeaponAnimationState::RELOADING && m_HUD_Weapon.AnimationIsPastPercentage(75))
			{
				m_HUDWeaponAnimationState = HUDWeaponAnimationState::FIRING;
				m_HUD_Weapon.PlayAnimation("Glock_Fire" + std::to_string(rand() % 3) + ".fbx");
				std::string file = "Glock_Fire_" + std::to_string(rand() % 3) + ".wav";
				Audio::PlayAudio(file.c_str(), 0.5f);

				FireGlock();
				SpawnGlockCasing();
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
		&& m_ammo_in_clip != m_clip_size
		&& m_ammo_total > 0)
	{
		m_HUD_Weapon.PlayAnimation("Glock_Reload.fbx");
		m_HUDWeaponAnimationState = HUDWeaponAnimationState::RELOADING;
		Audio::PlayAudio("Glock_Reload.wav", 0.5f);

		unsigned int ammo_to_add = std::min(m_clip_size - m_ammo_in_clip, m_ammo_total);
		m_ammo_in_clip += ammo_to_add;
		m_ammo_total -= ammo_to_add;
	}

	// Presses Equip
	if (Input::KeyPressed(HELL_KEY_Q) && m_HUDWeaponAnimationState != HUDWeaponAnimationState::EQUIPPING)
	{
		m_HUD_Weapon.PlayAnimation("Glock_FirstEquip1.fbx");
		m_HUDWeaponAnimationState = HUDWeaponAnimationState::EQUIPPING;
		Audio::PlayAudio("Glock_Equip.wav", 0.5f);
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
		m_HUD_Weapon.PlayAndLoopAnimation("Glock_Idle.fbx");
		m_HUDWeaponAnimationState = HUDWeaponAnimationState::IDLE;
	}



	//FireGlock();
}

/*template <class T>
bool IsPlayer(T* t) { return false; } // normal case returns false 

template <>
bool IsPlayer(Player* t) { return true; }*/


void Player::FireGlock()
{
	m_ammo_in_clip--;

	if (m_cameraRay.HitFound())
	{
		if (m_cameraRay.m_hitObjectName == "RAGDOLL"
			|| m_cameraRay.m_hitObjectName == "RAGDOLL_HEAD")
		{
			PxVec3 force = PxVec3(m_camera.m_Front.x, m_camera.m_Front.y, m_camera.m_Front.z) * 1000;

			PxRigidDynamic* actor = (PxRigidDynamic*)m_cameraRay.m_hitActor;
			actor->addForce(force);
			
			EntityData* physicsData = (EntityData*)actor->userData;

			// blood noise
			if (physicsData->type == PhysicsObjectType::RAGDOLL || physicsData->type == PhysicsObjectType::PLAYER_RAGDOLL) 
			{	
				std::string file = "FLY_Bullet_Impact_Flesh_0" + std::to_string(rand() % 8 + 1) + ".wav";
				Audio::PlayAudio(file.c_str(), 0.5f);
			}

			// kill em if it's player
			if (physicsData->type == PhysicsObjectType::PLAYER_RAGDOLL) {

				Player* p = (Player*)physicsData->parent;

				// kill em
				if (m_cameraRay.m_hitObjectName == "RAGDOLL_HEAD" && p->m_isAlive) {
					p->m_isAlive = false;
					m_killCount++;
					std::string file = "Death0.wav";
					Audio::PlayAudio(file.c_str(), 0.45f);
				}
			}
		}
	}
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
		// camera position (it's player plus view height)
		m_camera.m_transform.position = m_transform.position;
		m_camera.m_transform.position.y += m_cameraViewHeight;

		// weapon transform 
		Transform weaponTransform;
		weaponTransform.position = m_camera.m_transform.position;
		weaponTransform.rotation = m_camera.m_transform.rotation;
		weaponTransform.scale = glm::vec3(0.001);
		m_HUD_Weapon.SetTransform(weaponTransform);


		// glock node 102 is Camera001 //
		glm::mat4 animatedCameraMatrix = m_HUD_Weapon.m_animatedTransforms.cameraMatrix;


		// update matrices and return bish
		m_camera.CalculateMatrices(animatedCameraMatrix);


		return m_camera.m_viewMatrix;
	}
	else
	{
		for (RigidComponent& rigid : m_ragdoll.m_rigidComponents) 
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