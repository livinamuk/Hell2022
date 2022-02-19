#pragma once
#include "Header.h"
#include "Core/Camera.h"
#include "Animation/SkinnedModel.h"
#include "Animation/AnimatedGameObject.h"
#include "Physics/RayCast.h"
#include "GameCharacter.h"
#include "Renderer/Material.h"
#include "Effects/BloodPool.h"

struct PlayerControls
{
	unsigned int WALK_FORWARD = HELL_KEY_W;
	unsigned int WALK_BACKWARD = HELL_KEY_S;
	unsigned int WALK_LEFT = HELL_KEY_A;
	unsigned int WALK_RIGHT = HELL_KEY_D;
	unsigned int INTERACT = HELL_KEY_E;
	unsigned int RELOAD = HELL_KEY_R;
	unsigned int FIRE = HELL_MOUSE_LEFT;
	unsigned int JUMP = HELL_KEY_SPACE;
	unsigned int CROUCH = HELL_KEY_LEFT_CONTROL;
};


class Player
{
public: // methods
	//void Update(float deltaTime);
	void UpdateCamera(int renderWidth, int renderHeight);
	void Update(float deltaTime);
	void Respawn();
	void FireCameraRay();
	void CheckForWeaponInput();
	void UpdateMovement(float deltaTime);
	void UpdateAiming();
	void SetPosition(glm::vec3 position);
	glm::vec3 GetPosition();
	glm::vec3 GetRotation();
	glm::mat4& GetCameraProjectionMatrix();
	glm::mat4& GetCameraViewMatrix();
	Camera* GetCameraPointer();
	bool IsMoving();
	bool IsCrouched();
	void RenderCharacterModel(Shader* shader);
	void SetCharacterModel(SkinnedModel* skinnedModel);
	void ForceRagdollToMatchAnimation();
	void FireGlock();
	void SpawnGlockCasing();
	void SetControlsToDefaultPS4Controls();
	void FootstepAudio(float deltaTime);
	int GetControllerIndex();
	void SetControllerIndex(int index);

	bool PressingWalkForward();
	bool PressingWalkBackward();
	bool PressingWalkLeft();
	bool PressingWalkRight();
	bool PressingCrouch();

	bool PressedInteract();
	bool PressedReload();
	bool PressedFire();
	bool PressingFire();
	bool PressedJump();
	bool PressedCrouch(); 

	void SpawnBloodPool();

	void CreateCharacterController();

	//void SetMaterial(Material* material);


	void UpdatePlayerModelAnimation(float deltaTime);

private:

public: // fields
	CharacterModelAnimationState m_characterModelAnimationState = CharacterModelAnimationState::STOPPED;
	HUDWeaponAnimationState m_HUDWeaponAnimationState = HUDWeaponAnimationState::IDLE;    
	
	bool m_isCrouching = false;
	bool m_hasHitFloorYet = false;
	void HitFloor();

	std::vector<GameCharacter>* p_gameCharacters = nullptr;
	std::vector<BloodPool>* p_bloodPools = nullptr;

	bool m_justBorn = true;
	bool m_createdBloodPool = false;

	bool m_enableControl = true;

	SkinnedModel* m_currentWeaponSkinnedModel;

	PxRaycastBuffer m_cameraRayHit;

	RayCast m_cameraRay;

	PlayerControls m_controls;
	InputType m_inputType = InputType::KEYBOARD_AND_MOUSE;
	ControllerStickMode m_leftStickMode = ControllerStickMode::MOVEMENT;
	ControllerStickMode m_rightStickMode = ControllerStickMode::AIMING;


	AnimatedGameObject m_HUD_Weapon;
	AnimatedGameObject m_character_model;

	unsigned int m_ammo_total = 80;
	unsigned int m_ammo_in_clip = 15;
	unsigned int m_clip_size = 15;

	float m_cameraViewHeight = 1.5f;
	float m_viewHeightStanding = 1.5f;
	float m_viewHeightCrouching = 1.0f;
	float m_crouchDownSpeed = 17.5f;

	unsigned int m_killCount = 0;

	PxController* m_characterController = nullptr;

private:
	Transform m_transform;
	Camera m_camera;
	bool m_isMoving = false;
	SkinnedModel* m_skinnedModel; 
	float m_footstepAudioTimer = 0;
	int m_controllerIndex = -1;

public:
	Ragdoll m_ragdoll;
	bool m_isAlive = true;
	float m_timeSinceDeath = 0;
	int m_materialIndex = 0;

	glm::vec3 m_debugPos;
	glm::mat4 m_debugMat;

};