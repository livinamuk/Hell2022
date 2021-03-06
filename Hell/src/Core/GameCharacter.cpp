#include "GameCharacter.h"
#include "Renderer/Texture.h"
#include "Helpers/AssetManager.h"

GameCharacter::GameCharacter()
{

}

GameCharacter::~GameCharacter()
{
}

void GameCharacter::RenderSkinnedModel(Shader* shader)
{   
    // SKIN THE MODEL

    // Resize the vectors to the right size 
    if (m_skinningMethod != SkinningMethod::LAST_ANIMATED_STATE) {
        m_animatedTransforms.Resize(m_skinnedModel->m_BoneInfo.size());
        //m_animatedTransforms.resize(m_skinnedModel->m_BoneInfo.size());
        //m_animatedDebugTransforms_Animated.resize(m_skinnedModel->m_BoneInfo.size());
    }

    glm::mat4 modelMatrix;

    if (m_skinningMethod == SkinningMethod::BINDPOSE)
    {
        modelMatrix = m_transform.to_mat4();
        m_skinnedModel->UpdateBoneTransformsFromBindPose(m_animatedTransforms.local, m_animatedTransforms.worldspace);
    }
    else if (m_skinningMethod == SkinningMethod::RAGDOLL) {
        modelMatrix = glm::mat4(1);
        m_skinnedModel->UpdateBoneTransformsFromRagdoll(m_animatedTransforms.local, m_animatedTransforms.worldspace, &m_ragdoll);
        //return;
    }
    else if (m_skinningMethod == SkinningMethod::ANIMATED)
    {
        modelMatrix = m_transform.to_mat4();

		static float testTime = 0;
		testTime += 0.1f;

		Animation* animation = m_skinnedModel->m_animations[0];
		m_skinnedModel->UpdateBoneTransformsFromAnimation(testTime, animation, m_animatedTransforms);
      
        // animation is updated in Update() function
    }
    else if (m_skinningMethod == LAST_ANIMATED_STATE)
    {
        modelMatrix = glm::mat4(1);
    }

   // return;

    shader->setInt("hasAnimation", true);

    for (unsigned int i = 0; i < m_animatedTransforms.local.size(); i++)
        shader->setMat4("skinningMats[" + std::to_string(i) + "]", modelMatrix * m_animatedTransforms.local[i]);

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

void GameCharacter::Update(float deltaTime)
{
    m_lifeTimeInSeconds += deltaTime;

    // Optimisation for phasing out old ragdolls
    // any that are no longer moving or have been in existence for more than 10 seconds
    if (m_skinningMethod == SkinningMethod::RAGDOLL)
    {
        bool isAtRest = true;

        for (auto& r : m_ragdoll.m_rigidComponents) {
            if (!r.pxRigidBody->isSleeping()) {
                isAtRest = false;
                break;
            }
        }

        if (isAtRest || m_lifeTimeInSeconds > 10.0f) {
            m_skinningMethod = SkinningMethod::LAST_ANIMATED_STATE;
            m_ragdoll.RemovePhysicsObjects();
        }
    }

    if (m_skinningMethod == ANIMATED)
    {

    }
}

void GameCharacter::UpdateAnimation(float deltaTime)
{
    return;

	static float testTime = 0;
    testTime += deltaTime;

	m_animTime += deltaTime;

    int animIndex = 2;

    m_skinnedModel->UpdateBoneTransformsFromAnimation(testTime, animIndex, m_animatedTransforms.local, m_animatedTransforms.worldspace);
    ForceRagdollToMatchAnimation();
}

void GameCharacter::BuildRagdoll()
{
   /* m_ragdoll.BuildFromJsonFile("ragdoll.json", m_transform, this, PhysicsObjectType::RAGDOLL);
    
    for (auto rigid : m_ragdoll.m_rigidComponents)
        rigid.pxRigidBody->wakeUp();*/
}

void GameCharacter::ForceRagdollToMatchAnimation()
{
    for (RigidComponent& rigidComponent : m_ragdoll.m_rigidComponents)
    {
        int index = m_skinnedModel->m_BoneMapping[rigidComponent.correspondingJointName];  
        PxMat44 matrix = Util::GlmMat4ToPxMat44(m_transform.to_mat4() * m_animatedTransforms.worldspace[index]);
        rigidComponent.pxRigidBody->setGlobalPose(PxTransform(matrix));
        rigidComponent.pxRigidBody->putToSleep();
    }
}

void GameCharacter::HitFloor()
{
    if (!m_hasHitFloorYet)
    {
        m_hasHitFloorYet = true;

        std::string file = "BodyFall_01.wav";
        Audio::PlayAudio(file.c_str(), 0.5f);
    }    
}
