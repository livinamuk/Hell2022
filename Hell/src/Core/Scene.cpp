#include "Scene.h"
#include "Helpers/FileImporter.h"
#include "Helpers/AssetManager.h"

std::vector<GameCharacter> Scene::s_gameCharacters;
std::vector<BloodPool> Scene::s_bloodPools;

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

    s_gameCharacters.push_back(gameCharacter);  

    GameCharacter* g = &s_gameCharacters[s_gameCharacters.size() - 1];

    for (RigidComponent& rigid : g->m_ragdoll.m_rigidComponents) {
        rigid.pxRigidBody->wakeUp();
        rigid.pxRigidBody->userData = new EntityData(PhysicsObjectType::RAGDOLL, g);
    }

}

void Scene::RemoveCorpse()
{
    if (!s_gameCharacters.size())
        return;

    s_gameCharacters[0].m_ragdoll.RemovePhysicsObjects();
    s_gameCharacters.erase(s_gameCharacters.begin());
}
