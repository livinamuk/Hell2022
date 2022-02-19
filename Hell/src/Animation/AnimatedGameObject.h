#pragma once
#include "Header.h"
#include "SkinnedModel.h"
#include "Renderer/Shader.h"

class AnimatedGameObject
{
public:
	void SetSkinnedModel(SkinnedModel* skinnedModel);
	void PlayAnimation(std::string name);
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
	void Render(Shader* shader);

	float GetCurrentAnimationTime();
	float GetAnimationDuration();
	SkinnedModel* GetSkinnedModelPtr();
	bool AnimationIsComplete();	
	bool AnimationIsPastPercentage(float percent);
	Transform GetTransform();
	Animation* GetCurrentAnimation();

	glm::mat4* m_cameraMatrixPointer;

private:
	Transform m_transform;
	SkinnedModel* m_skinnedModel;
	Animation* m_animation;
	bool m_loopAnimation;
	bool m_pause;
	bool m_animationHasFinished;
	float m_animTime;
	//int m_animIndex;

public:
	
	AnimatedTransforms m_animatedTransforms;

	//std::vector<glm::mat4> m_animatedTransforms;
	//std::vector<glm::mat4> m_animatedTransformsWorldSpace;
};