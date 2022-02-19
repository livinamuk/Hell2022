#pragma once
#include "header.h"
#include "Animation/SkinnedModel.h"
#include "Physics/Ragdoll.h"

enum SkinningMethod { RAGDOLL, BINDPOSE, ANIMATED};

class GameCharacter
{
public:
    GameCharacter();
    ~GameCharacter();

  //  void SkinFromBindpose();
  //  void SkinFromRagdoll();
    void RenderSkinnedModel(Shader* shader);
    void UpdateAnimation(float deltaTime);
    void BuildRagdoll();
    void ForceRagdollToMatchAnimation();

    bool m_hasHitFloorYet = false;
    void HitFloor();

    int m_animIndex = 0;
    float m_animTime = 0;
    int m_materialIndex = 0;

    Transform m_transform;
    Ragdoll m_ragdoll;
    SkinnedModel* m_skinnedModel;
    SkinningMethod m_skinningMethod;

    std::vector<glm::mat4> m_animatedTransforms; 
    std::vector<glm::mat4> m_animatedDebugTransforms_Animated;

private: 
   // void FindBindPoseTransforms(const aiNode* pNode);
};