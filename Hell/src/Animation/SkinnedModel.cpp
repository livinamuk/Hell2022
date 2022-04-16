
#include "SkinnedModel.h"
#include "Helpers/Util.h"
//#include "Helpers/AssetManager.h"
//#include "Logic/WeaponLogic.h"
#include "Renderer/Renderer.h"

SkinnedModel::SkinnedModel()
{
    m_VAO = 0;
    ZERO_MEM(m_Buffers);
    m_NumBones = 0;
}

SkinnedModel::~SkinnedModel()
{
}

void SkinnedModel::Render(Shader* shader, const glm::mat4& modelMatrix, int materialIndex)
{
    glBindVertexArray(m_VAO);
    shader->setMat4("model", modelMatrix);

    for (int i = 0; i < m_meshEntries.size(); i++) {       

        if (materialIndex == 0)
            if (m_meshEntries[i].material)
               m_meshEntries[i].material->Bind();
        else if (materialIndex == 1)
            if (m_meshEntries[i].materialB)
                m_meshEntries[i].materialB->Bind();

        glDrawElementsBaseVertex(GL_TRIANGLES, m_meshEntries[i].NumIndices, GL_UNSIGNED_INT, (void*)(sizeof(unsigned int) * m_meshEntries[i].BaseIndex), m_meshEntries[i].BaseVertex);
    }
}

void SkinnedModel::UpdateBoneTransformsFromRagdoll(std::vector<glm::mat4>& Transforms, std::vector<glm::mat4>& DebugAnimatedTransforms, Ragdoll* ragdoll)
{
    // Traverse the tree 
    for (int i = 0; i < m_joints.size(); i++)
    {
        // Get the node and its um bind pose transform?
        const char* NodeName = m_joints[i].m_name;
        glm::mat4 NodeTransformation = m_joints[i].m_inverseBindTransform;

        unsigned int parentIndex = m_joints[i].m_parentIndex;

        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : m_joints[parentIndex].m_currentFinalTransform;
        glm::mat4 GlobalTransformation = ParentTransformation * NodeTransformation;

        m_joints[i].m_currentFinalTransform = GlobalTransformation;

        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            unsigned int BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
            m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;

            if (ragdoll != NULL)
            {
                //std::cout << "ragdol rigid size:  " << ragdoll->m_jointComponents.size() << "\n";
                //std::cout << "fuck " << NodeName << "\n";
                RigidComponent* rigidComponent = ragdoll->GetRigidByName(NodeName);


                if (rigidComponent != NULL) {

                    PxRigidDynamic* pxRigidBody = rigidComponent->pxRigidBody;

                    PxMat44 shapePose = pxRigidBody->getGlobalPose();

                    glm::mat4 matrix = Util::PxMat44ToGlmMat4(shapePose);

                    m_BoneInfo[BoneIndex].FinalTransformation = matrix * m_BoneInfo[BoneIndex].BoneOffset;
                    m_joints[i].m_currentFinalTransform = matrix;
                    m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = matrix;
                }
            }
        }
    }

    Transforms.resize(m_joints.size());
    DebugAnimatedTransforms.resize(m_joints.size());

    for (unsigned int i = 0; i < m_NumBones; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
        DebugAnimatedTransforms[i] = m_BoneInfo[i].ModelSpace_AnimatedTransform;
    }
}

void SkinnedModel::UpdateBoneTransformsFromBindPose(std::vector<glm::mat4>& Transforms, std::vector<glm::mat4>& DebugAnimatedTransforms)
{
    // Traverse the tree 
    for (int i = 0; i < m_joints.size(); i++)
    {
        // Get the node and its um bind pose transform?
        const char* NodeName = m_joints[i].m_name;
        glm::mat4 NodeTransformation = m_joints[i].m_inverseBindTransform;

        unsigned int parentIndex = m_joints[i].m_parentIndex;

        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : m_joints[parentIndex].m_currentFinalTransform;
        glm::mat4 GlobalTransformation = ParentTransformation * NodeTransformation;

        m_joints[i].m_currentFinalTransform = GlobalTransformation;

        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            unsigned int BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
            m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;          
        }
    }

    Transforms.resize(m_joints.size());
    DebugAnimatedTransforms.resize(m_joints.size());

    for (unsigned int i = 0; i < m_NumBones; i++) {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
        DebugAnimatedTransforms[i] = m_BoneInfo[i].ModelSpace_AnimatedTransform;
    }
}

int SkinnedModel::FindAnimatedNodeIndex(float AnimationTime, const AnimatedNode* animatedNode)
{
    // bail if current animation time is earlier than the this nodes first keyframe time
    if (AnimationTime < animatedNode->m_nodeKeys[0].timeStamp)
        return -1;

    for (unsigned int i = 0; i < animatedNode->m_nodeKeys.size() - 1; i++) {
        if (AnimationTime < animatedNode->m_nodeKeys[i + 1].timeStamp)
            return i;
    }
    return -1;
}


void SkinnedModel::CalcInterpolatedPosition(glm::vec3& Out, float AnimationTime, const AnimatedNode* animatedNode)
{
    int PositionIndex = FindAnimatedNodeIndex(AnimationTime, animatedNode);
    int NextPositionIndex = (PositionIndex + 1);

    // Nothing to report
    if (PositionIndex == -1 || animatedNode->m_nodeKeys.size() == 1) {
        Out = animatedNode->m_nodeKeys[0].positon;
        return;
    }       
    float DeltaTime = animatedNode->m_nodeKeys[NextPositionIndex].timeStamp - animatedNode->m_nodeKeys[PositionIndex].timeStamp;
    float Factor = (AnimationTime - animatedNode->m_nodeKeys[PositionIndex].timeStamp) / DeltaTime;

    glm::vec3 start = animatedNode->m_nodeKeys[PositionIndex].positon;
    glm::vec3 end = animatedNode->m_nodeKeys[NextPositionIndex].positon;
    glm::vec3 delta = end - start;
    Out = start + Factor * delta;
}


void SkinnedModel::CalcInterpolatedRotation(glm::quat& Out, float AnimationTime, const AnimatedNode* animatedNode)
{
    int Index = FindAnimatedNodeIndex(AnimationTime, animatedNode);
    int NextIndex = (Index + 1);

    // Nothing to report
    if (Index == -1 || animatedNode->m_nodeKeys.size() == 1) {
        Out = animatedNode->m_nodeKeys[0].rotation;
        return;
    }
    float DeltaTime = animatedNode->m_nodeKeys[NextIndex].timeStamp - animatedNode->m_nodeKeys[Index].timeStamp;
    float Factor = (AnimationTime - animatedNode->m_nodeKeys[Index].timeStamp) / DeltaTime;

    const glm::quat& StartRotationQ = animatedNode->m_nodeKeys[Index].rotation;
    const glm::quat& EndRotationQ = animatedNode->m_nodeKeys[NextIndex].rotation;

    Util::InterpolateQuaternion(Out, StartRotationQ, EndRotationQ, Factor);
    Out = glm::normalize(Out);
}


void SkinnedModel::CalcInterpolatedScaling(glm::vec3& Out, float AnimationTime, const AnimatedNode* animatedNode)
{
    int Index = FindAnimatedNodeIndex(AnimationTime, animatedNode);
    int NextIndex = (Index + 1);

    // Nothing to report
    if (Index == -1 || animatedNode->m_nodeKeys.size() == 1) {
        Out = glm::vec3(animatedNode->m_nodeKeys[0].scale);
        return;
    }
    float DeltaTime = animatedNode->m_nodeKeys[NextIndex].timeStamp - animatedNode->m_nodeKeys[Index].timeStamp;
    float Factor = (AnimationTime - animatedNode->m_nodeKeys[Index].timeStamp) / DeltaTime;

    glm::vec3 start = glm::vec3(animatedNode->m_nodeKeys[Index].scale);
    glm::vec3 end = glm::vec3(animatedNode->m_nodeKeys[NextIndex].scale);
    glm::vec3 delta = end - start;
    Out = start + Factor * delta;
}


void SkinnedModel::UpdateBoneTransformsFromAnimation(float animTime, Animation* animation, AnimatedTransforms& animatedTransforms)
{
    // Get the animation time
    float AnimationTime = 0;
    if (m_animations.size() > 0) {
        float TicksPerSecond = animation->m_ticksPerSecond != 0 ? animation->m_ticksPerSecond : 25.0f;
        // float TimeInTicks = TimeInSeconds * TicksPerSecond; chek thissssssssssssss could be a seconds thing???
        float TimeInTicks = animTime * TicksPerSecond;
        AnimationTime = fmod(TimeInTicks, animation->m_duration);
    }

    // Traverse the tree 
    for (int i = 0; i < m_joints.size(); i++)
    {
        // Get the node and its um bind pose transform?
        const char* NodeName = m_joints[i].m_name;
        glm::mat4 NodeTransformation = m_joints[i].m_inverseBindTransform;

        // Calculate any animation
        if (m_animations.size() > 0)
        {
            const AnimatedNode* animatedNode = FindAnimatedNode(animation, NodeName);

            if (animatedNode)
            {
                glm::vec3 Scaling;
                CalcInterpolatedScaling(Scaling, AnimationTime, animatedNode);
                glm::mat4 ScalingM;

                ScalingM = Util::Mat4InitScaleTransform(Scaling.x, Scaling.y, Scaling.z);
                glm::quat RotationQ;
                CalcInterpolatedRotation(RotationQ, AnimationTime, animatedNode);
                glm::mat4 RotationM(RotationQ);

                glm::vec3 Translation;
                CalcInterpolatedPosition(Translation, AnimationTime, animatedNode);
                glm::mat4 TranslationM;

                TranslationM = Util::Mat4InitTranslationTransform(Translation.x, Translation.y, Translation.z);
                NodeTransformation = TranslationM * RotationM * ScalingM;
            }
        }
        unsigned int parentIndex = m_joints[i].m_parentIndex;

        glm::mat4 ParentTransformation = (parentIndex == -1) ? glm::mat4(1) : m_joints[parentIndex].m_currentFinalTransform;
        glm::mat4 GlobalTransformation = ParentTransformation * NodeTransformation;

        // Store the current transformation, so child nodes can access it
        m_joints[i].m_currentFinalTransform = GlobalTransformation;

      //  std::cout << i << " " << NodeName << "\n";

       if (Util::StrCmp(NodeName, "Camera001") || Util::StrCmp(NodeName, "Camera")) {
            animatedTransforms.cameraMatrix = GlobalTransformation;
            //std::cout << i << " found it /n";
       //     Util::PrintMat4(animatedTransforms.cameraMatrix);
        }

        if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
            unsigned int BoneIndex = m_BoneMapping[NodeName];
            m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
            m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;

            // If there is no bind pose, then just use bind pose
            // ???? How about you check if this does anything useful ever ????
            if (m_animations.size() == 0) {
                m_BoneInfo[BoneIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[BoneIndex].BoneOffset;
                m_BoneInfo[BoneIndex].ModelSpace_AnimatedTransform = GlobalTransformation;
            }
        }
    }

    animatedTransforms.Resize(m_NumBones);

    for (unsigned int i = 0; i < m_NumBones; i++) {
        animatedTransforms.local[i] = m_BoneInfo[i].FinalTransformation;
        animatedTransforms.worldspace[i] = m_BoneInfo[i].ModelSpace_AnimatedTransform;
       // Transforms[i] = m_BoneInfo[i].FinalTransformation;
       // DebugAnimatedTransforms[i] = m_BoneInfo[i].ModelSpace_AnimatedTransform;
    }
}

void SkinnedModel::UpdateBoneTransformsFromAnimation(float animTime, int animationIndex, std::vector<glm::mat4>& Transforms, std::vector<glm::mat4>& DebugAnimatedTransforms)
{
    Animation* animation = m_animations[animationIndex];
   // UpdateBoneTransformsFromAnimation(animTime, animation, Transforms, DebugAnimatedTransforms);
}

const AnimatedNode* SkinnedModel::FindAnimatedNode(Animation* animation, const char* NodeName)
{
    for (unsigned int i = 0; i < animation->m_animatedNodes.size(); i++) {
        const AnimatedNode* animatedNode = &animation->m_animatedNodes[i];

        if (Util::StrCmp(animatedNode->m_nodeName, NodeName)) {
            return animatedNode;
        }
    }
    return nullptr;
}