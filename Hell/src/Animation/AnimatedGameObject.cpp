#include "AnimatedGameObject.h"
#include "Helpers/AssetManager.h"

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

	m_blendFrameTransforms = m_animatedTransforms;
	m_blend = true;

	// otherwise find the animation and begin playing it from the beginning
	for (Animation* animation : m_skinnedModel->m_animations)
	{
		// store the last skinned frame to use later for blending
		

		if (animation->m_filename == name) {
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
		}
	}
	// Now finally calculate the transforms the cunt
	if (skin) {
		//m_skinnedModel->UpdateBoneTransformsFromAnimation(m_animTime, m_animation, m_animatedTransforms);

		CalculateBoneTransforms();

		// blend the cunt
		/*if (m_blend) {
			float factor = 1 - (m_animTime * 4);
			BlendWithSingleKeyFrame(factor);

			std::cout << m_animTime << "\n";

			if (factor < 0)
				m_blend = false;
		}*/
	}
}

void AnimatedGameObject::Render(Shader* shader, glm::mat4 modelMatrix)
{
	shader->use();
	shader->setBool("hasAnimation", true);




	//glm::mat4 modelMatrix = glm::mat4(1);

	for (unsigned int i = 0; i < m_animatedTransforms.local.size(); i++)
		shader->setMat4("skinningMats[" + std::to_string(i) + "]", modelMatrix * m_animatedTransforms.local[i]);

	m_skinnedModel->Render(shader, m_transform.to_mat4());
	
	

	//SkinnedModel* femaleArms = AssetManager::GetSkinnedModelPtr("FemaleArms");
	//femaleArms->Render(shader, m_transform.to_mat4());

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

void AnimatedGameObject::BlendWithSingleKeyFrame(float factor)
{
	if (factor <= 0)
		return;

	factor = std::min(factor, 1.0f);

	for (int i=0; i < m_animatedTransforms.local.size(); i++)
	{
		glm::mat4& transformationA = m_animatedTransforms.local[i];
		glm::mat4& transformationB = m_blendFrameTransforms.local[i];

		glm::vec3 scaleA;
		glm::quat rotationA;
		glm::vec3 translationA;
		glm::vec3 scaleB;
		glm::quat rotationB;
		glm::vec3 translationB;

		glm::vec3 skew;
		glm::vec4 perspective;

		glm::decompose(transformationA, scaleA, rotationA, translationA, skew, perspective);
		glm::decompose(transformationB, scaleB, rotationB, translationB, skew, perspective);

		glm::vec3 translation = BlendVec3(translationA, translationB, factor);
		glm::quat rotation = BlendQuat(rotationA, rotationB, factor);

		glm::mat4 m = glm::translate(glm::mat4(1), translation);
		glm::quat qt = glm::quat(rotation);
		m *= glm::mat4_cast(qt);
		m = glm::scale(m, scaleA);

		m_animatedTransforms.local[i] = m;
	}
}

void AnimatedGameObject::CalculateBoneTransforms()
{
	if (!m_skinnedModel->m_animations.size()) {
		std::cout << "This skinned model has no animations\n";
		return;
	}

	m_SQTs.resize(m_skinnedModel->m_NumBones);
	m_animatedTransforms.Resize(m_skinnedModel->m_NumBones);

	// Get the animation time
	float AnimationTime = 0;
	if (m_skinnedModel->m_animations.size() > 0) {
		float TicksPerSecond = m_animation->m_ticksPerSecond != 0 ? m_animation->m_ticksPerSecond : 25.0f;
		// float TimeInTicks = TimeInSeconds * TicksPerSecond; chek thissssssssssssss could be a seconds thing???
		float TimeInTicks = m_animTime * TicksPerSecond;
		AnimationTime = fmod(TimeInTicks, m_animation->m_duration);
		AnimationTime = std::min(TimeInTicks, m_animation->m_duration);
	}

	// Traverse the tree 
	for (int i = 0; i < m_skinnedModel->m_joints.size(); i++)
	{
		// Get the node and its um bind pose transform?
		const char* NodeName = m_skinnedModel->m_joints[i].m_name;
		glm::mat4 NodeTransformation = m_skinnedModel->m_joints[i].m_inverseBindTransform;

		const AnimatedNode* animatedNode = m_skinnedModel->FindAnimatedNode(m_animation, NodeName);

		SQT sqt;

		if (animatedNode)
		{
			glm::vec3 Scaling;
			m_skinnedModel->CalcInterpolatedScaling(Scaling, AnimationTime, animatedNode);
			glm::mat4 ScalingM;

			ScalingM = Util::Mat4InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);
			glm::quat RotationQ;
			m_skinnedModel->CalcInterpolatedRotation(RotationQ, AnimationTime, animatedNode);
			glm::mat4 RotationM(RotationQ);

			glm::vec3 Translation;
			m_skinnedModel->CalcInterpolatedPosition(Translation, AnimationTime, animatedNode);
			glm::mat4 TranslationM;

			TranslationM = Util::Mat4InitTranslationTransform(Translation.x, Translation.y, Translation.z);
			NodeTransformation = TranslationM * RotationM * ScalingM;

			sqt.scale = Scaling.x;
			sqt.positon = Translation;
			sqt.rotation = RotationQ;
		}
		
		unsigned int parentIndex = m_skinnedModel->m_joints[i].m_parentIndex;

		glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : m_skinnedModel->m_joints[parentIndex].m_currentFinalTransform;
		glm::mat4 GlobalTransformation = ParentTransformation * NodeTransformation;

		// Store the current transformation, so child nodes can access it
		m_skinnedModel->m_joints[i].m_currentFinalTransform = GlobalTransformation;

		if (Util::StrCmp(NodeName, "Camera001") || Util::StrCmp(NodeName, "Camera")) {
			m_animatedTransforms.cameraMatrix = GlobalTransformation;
			glm::mat4 cameraMatrix = GlobalTransformation;
			cameraMatrix[3][0] = 0;
			cameraMatrix[3][1] = 0;
			cameraMatrix[3][2] = 0;
			m_animatedTransforms.cameraMatrix = cameraMatrix;
		}

	
		// Dose this animated node have a matching bone in the model?
		if (m_skinnedModel->m_BoneMapping.find(NodeName) != m_skinnedModel->m_BoneMapping.end()) 
		{
			unsigned int BoneIndex = m_skinnedModel->m_BoneMapping[NodeName];
			m_animatedTransforms.local[BoneIndex] = GlobalTransformation * m_skinnedModel->m_BoneInfo[BoneIndex].BoneOffset;
			m_animatedTransforms.worldspace[BoneIndex] = GlobalTransformation;

			m_SQTs[BoneIndex] = sqt;
		}
	}
}

glm::vec3 AnimatedGameObject::BlendVec3(glm::vec3& a, glm::vec3& b, float factor)
{
	glm::vec3 delta = b - a;
	return a + factor * delta;
}

glm::quat AnimatedGameObject::BlendQuat(glm::quat& a, glm::quat& b, float factor)
{
	glm::quat result;
	Util::InterpolateQuaternion(result, a, b, factor);
	return glm::normalize(result);
}
