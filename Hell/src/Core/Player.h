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
	unsigned int NEXT_WEAPON = HELL_KEY_Q;
};


class Player
{
public: // methods


	enum class Gun {
		NONE, GLOCK, SHOTGUN
	}; 
	
	enum class ShotgunReloadState { NOT_RELOADING, FROM_IDLE, SINGLE_RELOAD, DOUBLE_RELOAD, BACK_TO_IDLE };


	//void Update(float deltaTime);
	void Interact();
	void UpdateCamera(int renderWidth, int renderHeight);
	void Update(float deltaTime);
	void Respawn();
	void FireCameraRay();
	void FireBulletRay(float variance);
	void FireBulletRay(glm::vec3 unitDir);
	void CheckForWeaponInput();
	void UpdateMovement(float deltaTime);
	void UpdateAiming();
	void SetPosition(glm::vec3 position);
	glm::vec3 GetPosition();
	glm::vec3 GetRotation();
	glm::mat4& GetCameraProjectionMatrix();
	glm::mat4& GetCameraViewMatrix();
	glm::vec3 GetViewPos();
	Camera* GetCameraPointer();
	bool IsMoving();
	bool IsCrouched();
	void RenderCharacterModel(Shader* shader);
	void SetCharacterModel(SkinnedModel* skinnedModel);
	void ForceRagdollToMatchAnimation();
	void FireGlock();
	void FireShotgun();
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
	bool PressedNextWeapon();

	void SpawnBloodPool();
	void SpawnMuzzleFlash();

	void CreateCharacterController();
	void SpawnCoprseRagdoll();

	//void SetMaterial(Material* material);


	void UpdatePlayerModelAnimation(float deltaTime);

	void CreatePoormansCharacterController();

	int GetCurrentGunAmmoInClip();
	int GetCurrentGunTotalAmmo();

	void FireBullet(float variance, float force);

private:

public: // fields
	CharacterModelAnimationState m_characterModelAnimationState = CharacterModelAnimationState::STOPPED;
	HUDWeaponAnimationState m_HUDWeaponAnimationState = HUDWeaponAnimationState::EQUIPPING;    
	
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
	RayCast m_bulletRay;

	PlayerControls m_controls;
	InputType m_inputType = InputType::KEYBOARD_AND_MOUSE;
	ControllerStickMode m_leftStickMode = ControllerStickMode::MOVEMENT;
	ControllerStickMode m_rightStickMode = ControllerStickMode::AIMING;

	GameCharacter* m_corpse = nullptr;


	AnimatedGameObject m_HUD_Weapon;
	AnimatedGameObject m_character_model;

	float m_cameraViewHeight = 1.5f;
	float m_viewHeightStanding = 1.5f;
	float m_viewHeightCrouching = 1.0f;
	float m_crouchDownSpeed = 17.5f;

	unsigned int m_killCount = 0;

	PxController* m_characterController = nullptr;

	Camera m_camera;

	float m_aim_trigger_axis_X;
	float m_aim_trigger_axis_Y;

	Gun m_gun = Gun::GLOCK;
	Gun m_gunToChangeTo = Gun::NONE;
	ShotgunReloadState m_shotgunReloadState = ShotgunReloadState::NOT_RELOADING;

private:
	unsigned int m_ammo_glock_total = 80;
	unsigned int m_ammo_glock_in_clip = 15;
	unsigned int m_clip_size_glock = 15;

	unsigned int m_ammo_shotgun_total = 48;
	unsigned int m_ammo_shotgun_in_clip = 8;
	unsigned int m_clip_size_shotgun = 8;

	Transform m_transform;
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

	float m_muzzleFlashTimer = -1;
	float m_muzzleFlashRotation = 0;

	PxRigidDynamic* m_rigid;

	bool m_firstShellLoaded = false;
	bool m_secondShellLoaded = false;
};