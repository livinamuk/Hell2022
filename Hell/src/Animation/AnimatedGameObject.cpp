#include "AnimatedGameObject.h"

void AnimatedGameObject::SetSkinnedModel(SkinnedModel* skinnedModel)
{
	m_skinnedModel = skinnedModel;

	if (m_skinnedModel->m_animations.size())
		m_animation = m_skinnedModel->m_animations[0];
}

void AnimatedGameObject::PlayAnimation(std::string name, float speed)
{
	m_animationHasFinished = false;

	if (!m_skinnedModel)
		return;

	for (Animation* animation : m_skinnedModel->m_animations) 
	{
		if (animation->m_filename == name)
		{
			m_animation = animation;
			m_animTime = 0;
			m_loopAnimation = false;
			m_animSpeed = speed;
			return;
		}
	}
}

void AnimatedGameObject::PlayAndLoopAnimation(std::string name)
{
	if (!m_skinnedModel)
		return;

	// if the current animation is already playing then don't change anything
	if (m_animation && m_animation->m_filename == name) {
		m_loopAnimation = true;
		return;
	}
		
	// otherwise find the animation and begin playing it from the beginning
	for (Animation* animation : m_skinnedModel->m_animations)
	{
		if (animation->m_filename == name)
		{
			m_animation = animation;
			m_animTime = 0;
			m_loopAnimation = true;
			return;
		}
	}
}

void AnimatedGameObject::PlayAndLoopAnimationIfItIsNotAlreadyPlaying(std::string name)
{
	
		PlayAndLoopAnimation(name);
}

void AnimatedGameObject::PauseAnimation()
{
	m_pause = true;
}

void AnimatedGameObject::UnpauseAnimation()
{
	m_pause = false;
}

void AnimatedGameObject::SetTransform(Transform transform)
{
	m_transform = transform;
}

void AnimatedGameObject::SetPosition(glm::vec3 position)
{
	m_transform.position = position;
}

void AnimatedGameObject::SetRotation(glm::vec3 rotation)
{
	m_transform.rotation = rotation;
}

void AnimatedGameObject::SetScale(glm::vec3 scale)
{
	m_transform.scale = scale;
}

void AnimatedGameObject::SetAnimationTime(float time)
{
	m_animTime = time;
}

#include "Core/Input.h"

void AnimatedGameObject::UpdateAnmation(float deltaTime, bool skin)
{
	m_animTime += deltaTime * m_animSpeed;

	//m_animTime = m_animation->m_duration * m_animation->m_ticksPerSecond

	// Check if animation has finished
	m_animationHasFinished = false;
	if (m_animTime * m_animation->GetTicksPerSecond() > m_animation->m_duration)
	{
		// If looping then start over
		if (m_loopAnimation) {
			m_animTime = 0;
			m_animSpeed = 1;	// this probably doesn't actually solve the bug, you may still get 1 play through at the speed at which whatever the last anim was played when u switch to a loooping one. check it by setting shotgun equip to 100 speed and then walking out of it
		}
		// If not then stop on final frame
		else {

			m_animTime = m_animation->m_duration / m_animation->GetTicksPerSecond();
			m_animationHasFinished = true;
			m_animSpeed = 1;

	//		std::cout << "m_filename: " << m_animation->m_filename << "\n";
////	std::cout << "m_animTime: " << m_animTime << "\n";
	//		std::cout << "m_duration / m_animation->GetTicksPerSecond(): " << (m_animation->m_duration / m_animation->GetTicksPerSecond()) << "\n";
		}
	}
	// Now finally skin the cunt
	if (skin)
		m_skinnedModel->UpdateBoneTransformsFromAnimation(m_animTime, m_animation, m_animatedTransforms);
}

void AnimatedGameObject::Render(Shader* shader, glm::mat4 modelMatrix)
{
	shader->use();
	shader->setBool("hasAnimation", true);




	//glm::mat4 modelMatrix = glm::mat4(1);

	for (unsigned int i = 0; i < m_animatedTransforms.local.size(); i++)
		shader->setMat4("skinningMats[" + std::to_string(i) + "]", modelMatrix * m_animatedTransforms.local[i]);

	m_skinnedModel->Render(shader, m_transform.to_mat4());

	shader->setBool("hasAnimation", false);
}

float AnimatedGameObject::GetCurrentAnimationTime()
{
	return m_animTime;
}

float AnimatedGameObject::GetAnimationDuration()
{
	return 0.0f;
}

SkinnedModel* AnimatedGameObject::GetSkinnedModelPtr()
{
	return m_skinnedModel;
}

bool AnimatedGameObject::AnimationIsComplete()
{
	return m_animationHasFinished;
}

bool AnimatedGameObject::AnimationIsPastPercentage(float percent)
{
	if (m_animTime * m_animation->GetTicksPerSecond() > m_animation->m_duration * (percent / 100.0))
		return true;
	else
		return false;
}

Transform AnimatedGameObject::GetTransform()
{
	return m_transform;
}

Animation* AnimatedGameObject::GetCurrentAnimation()
{
	return m_animation;
}
