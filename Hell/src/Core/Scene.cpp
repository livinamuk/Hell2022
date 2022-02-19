#include "Scene.h"
#include "Helpers/FileImporter.h"
#include "Helpers/AssetManager.h"

void Scene::Reset()
{
	//nurseModel = FileImporter::LoadSkinnedModel("NurseGuy.fbx");  
    
    // Remove any old characters and their ragdolls.
    while (m_gameCharacters.size() > 0) {
       // m_gameCharacters[0].m_ragdoll.RemovePhysicsObjects();
       // m_gameCharacters.erase(m_gameCharacters.begin());
    }

    // New test ragdoll
   /* GameCharacter gameCharacter;
    //gameCharacter.m_transform.position = glm::vec3(0, 0.25, -0.5f);
    gameCharacter.m_transform.position = glm::vec3(1, -1, 0.0f);
    gameCharacter.m_skinnedModel = AssetManager::GetSkinnedModelPtr("Nurse");
    gameCharacter.m_ragdoll.BuildFromJsonFile("ragdoll.json", gameCharacter.m_transform);
    gameCharacter.m_skinningMethod = SkinningMethod::ANIMATED;
    gameCharacter.m_animIndex = 1;

    // quick hack to fill debug transforms with non junk data
    gameCharacter.m_animatedDebugTransforms_Animated.resize(gameCharacter.m_skinnedModel->m_skeleton.m_joints.size());
    for (glm::mat4& m : gameCharacter.m_animatedDebugTransforms_Animated)
        m = glm::mat4(1);

    //gameCharacter.m_skinningMethod = SkinningMethod::BINDPOSE;
    m_gameCharacters.push_back(gameCharacter);
    */


}

void Scene::NewRagdoll()
{
    GameCharacter gameCharacter;
    gameCharacter.m_transform.position = glm::vec3(0, 5, 0);
    gameCharacter.m_transform.rotation.x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HELL_PI)));
    gameCharacter.m_transform.rotation.y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HELL_PI)));
    gameCharacter.m_transform.rotation.z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HELL_PI)));
    gameCharacter.m_skinnedModel = AssetManager::GetSkinnedModelPtr("Nurse");
    gameCharacter.m_ragdoll.BuildFromJsonFile("ragdoll.json", gameCharacter.m_transform, nullptr, PhysicsObjectType::RAGDOLL);
    gameCharacter.m_skinningMethod = SkinningMethod::RAGDOLL;

  //  for (auto rigid : gameCharacter.m_ragdoll.m_rigidComponents)
     //   rigid.pxRigidBody->wakeUp();

    m_gameCharacters.push_back(gameCharacter);  

    GameCharacter* g = &m_gameCharacters[m_gameCharacters.size() - 1];

    for (RigidComponent& rigid : g->m_ragdoll.m_rigidComponents) {
        rigid.pxRigidBody->wakeUp();
        rigid.pxRigidBody->userData = new EntityData(PhysicsObjectType::RAGDOLL, g);
    }

}

void Scene::AddEntity(Model* model, Material* material, Transform transform)
{
    if (model && material) {
        m_staticEntities.emplace_back(EntityStatic(model, material, transform));
    }
}