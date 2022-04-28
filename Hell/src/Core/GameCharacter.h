#pragma once
#include "header.h"
#include "Animation/SkinnedModel.h"
#include "Physics/Ragdoll.h"

enum SkinningMethod { RAGDOLL, BINDPOSE, ANIMATED, LAST_ANIMATED_STATE};

class GameCharacter
{
public:
  //  enum Type { UNDEFIND, CORPSE };

public:
    GameCharacter();
    ~GameCharacter();

  //  void SkinFromBindpose();
  //  void SkinFromRagdoll();
    void RenderSkinnedModel(Shader* shader);
    void Update(float deltaTime);
    void UpdateAnimation(float deltaTime);
    void BuildRagdoll();
    void ForceRagdollToMatchAnimation();
    bool m_hasHitFloorYet = false;
    void HitFloor();

    int m_animIndex = 0;
    float m_animTime = 0;
    int m_materialIndex = 0;

    float m_lifeTimeInSeconds = 0;

  //  Type m_type = Type::UNDEFIND;


    Transform m_transform;
    Ragdoll m_ragdoll;
    SkinnedModel* m_skinnedModel;
    SkinningMethod m_skinningMethod;

   // std::vector<glm::mat4> m_animatedTransforms; 
   // std::vector<glm::mat4> m_animatedDebugTransforms_Animated;

	AnimatedTransforms m_animatedTransforms;


private: 
   // void FindBindPoseTransforms(const aiNode* pNode);
};