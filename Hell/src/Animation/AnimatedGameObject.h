#pragma once
#include "Header.h"
#include "SkinnedModel.h"
#include "Renderer/Shader.h"

class AnimatedGameObject
{
public:
	void SetSkinnedModel(SkinnedModel* skinnedModel);
	void PlayAnimation(std::string name, float speed = 1.0f);
	void PlayAndLoopAnimation(std::string name);
	void PlayAndLoopAnimationIfItIsNotAlreadyPlaying(std::string name);
	void PauseAnimation();
	void UnpauseAnimation();
	void SetTransform(Transform transform);
	void SetPosition(glm::vec3 position);
	void SetRotation(glm::vec3 rotation);
	void SetScale(glm::vec3 scale);
	void SetAnimationTime(float time);
	void UpdateAnmation(float deltaTime, bool skin = true);
	void Render(Shader* shader, glm::mat4 modelMatrix = glm::mat4(1));

	float GetCurrentAnimationTime();
	float GetAnimationDuration();
	SkinnedModel* GetSkinnedModelPtr();
	bool AnimationIsComplete();	
	bool AnimationIsPastPercentage(float percent);
	Transform GetTransform();
	Animation* GetCurrentAnimation();


	void BlendWithSingleKeyFrame(float factor);
	void CalculateBoneTransforms();

	glm::vec3 BlendVec3(glm::vec3& a, glm::vec3& b, float factor);
	glm::quat BlendQuat(glm::quat& a, glm::quat& b, float factor);

	glm::mat4* m_cameraMatrixPointer;

private:
	Transform m_transform;
	SkinnedModel* m_skinnedModel;
	Animation* m_animation;
	bool m_loopAnimation;
	bool m_pause;
	bool m_animationHasFinished;
	float m_animTime;
	float m_animSpeed = 1;
	//int m_animIndex;

	bool m_blend = false;

	std::vector<SQT> m_SQTs;

public:
	
	AnimatedTransforms m_animatedTransforms;
	AnimatedTransforms m_blendFrameTransforms;



	//std::vector<glm::mat4> m_animatedTransforms;
	//std::vector<glm::mat4> m_animatedTransformsWorldSpace;
};