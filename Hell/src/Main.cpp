#include "Header.h"
#include "Core/CoreGL.h"
#include "Core/Input.h"
#include "Core/Camera.h"
#include "Core/Entity.h"
#include "Helpers/Util.h"
#include "Helpers/FileImporter.h"
#include "Renderer/Decal.h"
#include "Renderer/Model.h"
#include "Renderer/Texture.h"
#include "Renderer/TextBlitter.h"
#include "Renderer/Renderer.h"
#include "Audio/Audio.h"
#include "HellEngine.h"
#include "Core/GameData.h"
#include "Core/File.h"

int main()
{
    Input::s_showCursor = false;

    HellEngine hellEngine;
    hellEngine.Init();

    AssetManager::ForceLoadTexture("res/textures/CharSheet.png");

    while (AssetManager::AssetsToLoad() && !Input::s_keyDown[HELL_KEY_ESCAPE])
    {
        CoreGL::OnUpdate();
        Input::UpdateKeyboardInput(CoreGL::s_window);
        Input::HandleKeypresses();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, CoreGL::s_fullscreenWidth, CoreGL::s_fullscreenHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        TextBlitter::BlitLine(AssetManager::s_loadLog);

        Renderer::TextBlitterPass(&Renderer::s_textued_2D_quad_shader);
 
        glfwSwapBuffers(CoreGL::s_window);
        glfwPollEvents();

        AssetManager::LoadNextAssetToGL();
    }


    AssetManager::CreateMaterials();
    AssetManager::LoadSkinnedModel("Nurse", "NurseGuy.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Nurse"), "NurseGuyPistolIdle.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Nurse"), "NurseGuyPistolWalk.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Nurse"), "NurseGuyPistolCrouch.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Nurse"), "NursdeGuyShotgunIdle.fbx");

    AssetManager::LoadSkinnedModel("Glock", "Glock.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Glock"), "Glock_Idle.fbx"); 
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Glock"), "Glock_Fire0.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Glock"), "Glock_Fire1.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Glock"), "Glock_Fire2.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Glock"), "Glock_Walk.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Glock"), "Glock_Reload.fbx");
    FileImporter::LoadAnimation(AssetManager::GetSkinnedModelPtr("Glock"), "Glock_FirstEquip1.fbx");


    // set nurse guy mesh materials
    SkinnedModel* nurseModel = AssetManager::GetSkinnedModelPtr("Nurse");
    for (SkinnedModel::MeshEntry& meshEntry : nurseModel->m_meshEntries)
    {
        if (meshEntry.Name == "Ch16_Body1") {
            meshEntry.material = AssetManager::GetMaterialPtr("Nurse");
            meshEntry.materialB = AssetManager::GetMaterialPtr("NurseAlt");
        }
        if (meshEntry.Name == "Ch16_Cap") {
            meshEntry.material = AssetManager::GetMaterialPtr("Nurse2");
            meshEntry.materialB = AssetManager::GetMaterialPtr("Nurse2Alt");
        }
        if (meshEntry.Name == "Ch16_Eyelashes") {
            meshEntry.material = AssetManager::GetMaterialPtr("Nurse");
            meshEntry.materialB = AssetManager::GetMaterialPtr("NurseAlt");
        }
        if (meshEntry.Name == "Ch16_Mask") {
            meshEntry.material = AssetManager::GetMaterialPtr("Nurse2");
            meshEntry.materialB = AssetManager::GetMaterialPtr("Nurse2Alt");
        }
        if (meshEntry.Name == "Ch16_Pants") {
            meshEntry.material = AssetManager::GetMaterialPtr("Nurse");
            meshEntry.materialB = AssetManager::GetMaterialPtr("NurseAlt");
        }
        if (meshEntry.Name == "Ch16_Shirt") {
            meshEntry.material = AssetManager::GetMaterialPtr("Nurse");
            meshEntry.materialB = AssetManager::GetMaterialPtr("NurseAlt");
        }
        if (meshEntry.Name == "Ch16_Shoes") {
            meshEntry.material = AssetManager::GetMaterialPtr("Nurse2");
            meshEntry.materialB = AssetManager::GetMaterialPtr("Nurse2Alt");
        }
    }

    SkinnedModel* glockModel = AssetManager::GetSkinnedModelPtr("Glock");
    for (SkinnedModel::MeshEntry& meshEntry : glockModel->m_meshEntries)
    {
        // std::cout << meshEntry.Name << "\n";

        if (meshEntry.Name == "manniquen1_2")
            meshEntry.material = AssetManager::GetMaterialPtr("Hands");
        if (meshEntry.Name == "Glock")
            meshEntry.material = AssetManager::GetMaterialPtr("Glock");
        if (meshEntry.Name == "SM_40cal_01a")
            meshEntry.material = AssetManager::GetMaterialPtr("BulletCasing");
        if (meshEntry.Name == "SM_40cal_01a001")
            meshEntry.material = AssetManager::GetMaterialPtr("BulletCasing");
    }

    Audio::Init();


    Scene::AddEntity(AssetManager::GetModelPtr("Couch"), AssetManager::GetMaterialPtr("Couch"), Transform(glm::vec3(0, 0, -1)));

    Transform tableTransform;
    tableTransform.position = glm::vec3(-2, 1000, -1);
    tableTransform.position = glm::vec3(-2, 0, -1);
    tableTransform.rotation = glm::vec3(0, -0.5, 0);
 //   tableTransform.scale = glm::vec3(0.01);

   // Scene::AddEntity(AssetManager::GetModelPtr("TableSmall"), AssetManager::GetMaterialPtr("TableSmall"), tableTransform);



    GameData::s_player2.SetPosition(glm::vec3(0, 0, 0));
    GameData::s_player2.SetPosition(glm::vec3(0, 0, 2));
    GameData::s_player2.m_materialIndex = 1;
    GameData::s_player1.SetCharacterModel(AssetManager::GetSkinnedModelPtr("Nurse"));
    GameData::s_player2.SetCharacterModel(AssetManager::GetSkinnedModelPtr("Nurse"));
    GameData::s_player2.m_currentWeaponSkinnedModel = (AssetManager::GetSkinnedModelPtr("Glock"));
    GameData::s_player2.m_currentWeaponSkinnedModel = (AssetManager::GetSkinnedModelPtr("Glock"));

    GameData::s_player1.m_ragdoll.BuildFromJsonFile("ragdoll.json", GameData::s_player1.GetPosition(), &GameData::s_player1, PhysicsObjectType::PLAYER_RAGDOLL);
    GameData::s_player2.m_ragdoll.BuildFromJsonFile("ragdoll.json", GameData::s_player2.GetPosition(), &GameData::s_player2, PhysicsObjectType::PLAYER_RAGDOLL);

    GameData::s_player1.m_HUD_Weapon.SetSkinnedModel(AssetManager::GetSkinnedModelPtr("Glock"));
    GameData::s_player2.m_HUD_Weapon.SetSkinnedModel(AssetManager::GetSkinnedModelPtr("Glock"));
    GameData::s_player1.m_character_model.SetSkinnedModel(AssetManager::GetSkinnedModelPtr("Nurse"));
    GameData::s_player2.m_character_model.SetSkinnedModel(AssetManager::GetSkinnedModelPtr("Nurse"));


    GameData::s_player1.CreateCharacterController();


    hellEngine.m_currentPlayer = 1;

   // hellEngine.m_player1.m_HUD_Weapon.PlayAndLoopAnimation("Glock_Walk.fbx");

    //hellEngine.m_renderer.SetModelPointers();

    //hellEngine.couchEntity = Entity(, AssetManager::GetTexturePtr("Couch_ALB")->ID);
   // hellEngine.couchEntity.m_transform.position.z -= 1;
    //hellEngine.couchEntity.UpdateCollisionObject();

    ///////////////////////
    //   Main game loop  //

    double mouseX;
    double mouseY;
    glfwGetCursorPos(CoreGL::s_window, &mouseX, &mouseY);
    Input::m_xoffset = mouseX;
    Input::m_yoffset = mouseY;    
    Input::s_mouseX = mouseX;
    Input::s_mouseY = mouseY;

    CoreGL::SetVSync(1);

    double lastTime = glfwGetTime();
    double accumulator = 0.0;
    double fixedStep = 1.0 / 60.0;

    Door::CreateShapes();

    File::LoadMap("Map.json");

    GameData::s_lights.push_back(Light(glm::vec3(0, 2.2f, 0), DEFAULT_LIGHT_COLOR, 4, 10, 0.1, 0));
    GameData::s_lights.push_back(Light(glm::vec3(4, 2.2f, 3.5), DEFAULT_LIGHT_COLOR, 4, 10, 0.1, 0));
    GameData::s_lights.push_back(Light(glm::vec3(-3.9, 2.2f, 5), DEFAULT_LIGHT_COLOR, 4, 10, 0.1, 0));
    GameData::s_lights.push_back(Light(glm::vec3(-7.9, 2.2f, -1), glm::vec3(1,0,0), 4, 10, 0.1, 0));

   
  //  GameData::s_doors.push_back(Door(glm::vec3(0, 0, 2)));

    while (CoreGL::IsRunning() && !Input::s_keyDown[HELL_KEY_ESCAPE])
    {
        if (Input::KeyPressed(HELL_KEY_O))
            File::LoadMap("Map.json");

        double deltaTime = glfwGetTime() - lastTime;
        accumulator += deltaTime;

        // Update OpenGL: what does this do again? probably not stuff that needs its own function ya dickhead
        CoreGL::OnUpdate();
        Input::HandleKeypresses();

        // Update keyboard and controller states
        hellEngine.UpdateInput();




        if (!Editor::IsOpen())
        {
            
            hellEngine.Update(deltaTime);

            if (GameData::s_player1.m_isAlive)
                GameData::s_player1.ForceRagdollToMatchAnimation();

            if (GameData::s_player2.m_isAlive)
                GameData::s_player2.ForceRagdollToMatchAnimation();

            if (accumulator >= fixedStep)
            {

                //TextBlitter::BlitLine("current player: " + std::to_string(hellEngine.m_currentPlayer));

                for (RigidComponent& rigid : GameData::s_player1.m_ragdoll.m_rigidComponents)
                {
                    PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
                    PhysX::DisableRayCastingForShape(shape);
                }

                float x = GameData::s_player1.GetPosition().x;
                float y = GameData::s_player1.GetPosition().y + GameData::s_player1.m_cameraViewHeight;
                float z = GameData::s_player1.GetPosition().z;

                float Dx = GameData::s_player1.GetCameraPointer()->m_Front.x;
                float Dy = GameData::s_player1.GetCameraPointer()->m_Front.y;
                float Dz = GameData::s_player1.GetCameraPointer()->m_Front.z;
                PxVec3 origin = PxVec3(x, y, z);
                PxVec3 unitDir = PxVec3(Dx, Dy, Dz);

                PxScene* scene = PhysX::GetScene();
                //  PxVec3 origin = PxVec3(m_transform.position.x, m_transform.position.y + m_cameraViewHeight, m_transform.position.z);                 // [in] Ray origin
                //  PxVec3 unitDir = PxVec3(m_camera.m_Front.x, m_camera.m_Front.y, m_camera.m_Front.z);
                PxReal maxDistance = 10;

                PxRaycastBuffer hit;

                // [in] Define what parts of PxRaycastHit we're interested in
                //const PxHitFlags outputFlags = PxHitFlag::eDISTANCE | PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;
                const PxHitFlags outputFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

                // Only ray cast against objects with the GROUP_RAYCAST flag
                PxQueryFilterData filterData = PxQueryFilterData();
                filterData.data.word0 = GROUP_RAYCAST;

                bool status = scene->raycast(origin, unitDir, maxDistance, hit, outputFlags, filterData);



                for (RigidComponent& rigid : GameData::s_player1.m_ragdoll.m_rigidComponents)
                {
                    PxShape* shape = PhysX::GetShapeFromPxRigidDynamic(rigid.pxRigidBody);
                    PhysX::EnableRayCastingForShape(shape);
                }


                std::string hitname = "NONE";

                // glm::vec3 pos = glm::vec3(m_cameraRayHit.block.position.x, m_cameraRayHit.block.position.y, m_cameraRayHit.block.position.z);

                if (status)
                {
                    if (hit.block.actor->getName()) {
                        hitname = hit.block.actor->getName();
                    }
                    else
                        hitname = "HIT OBJECT HAS NO ACTOR NAME";
                }

                accumulator -= fixedStep;
            }

            TextBlitter::BlitLine("P1 RayCast: " + GameData::s_player1.m_cameraRay.m_hitObjectName);
            TextBlitter::BlitLine("P2 RayCast: " + GameData::s_player2.m_cameraRay.m_hitObjectName);
            TextBlitter::BlitLine("");
            TextBlitter::BlitLine("P1 Kill count: " + std::to_string(GameData::s_player1.m_killCount));
            TextBlitter::BlitLine("P2 Kill count: " + std::to_string(GameData::s_player2.m_killCount));
            TextBlitter::BlitLine("");
        

        }
    
        lastTime = glfwGetTime();



     //  hellEngine.m_camera.Update(1);

     //   hellEngine.m_camera.m_transform.rotation = glm::vec3(glfwGetTime() * 5, 0, 0);
     //   hellEngine.m_camera.CalculateMatrices();

        if (Editor::IsOpen()) 
        {
            Editor::Render(CoreGL::s_currentWidth, CoreGL::s_currentHeight);
        }
        else
            hellEngine.Render();
    
        
        glfwSwapBuffers(CoreGL::s_window);
        glfwPollEvents();


    }

    CoreGL::Terminate();
    return 0;
}