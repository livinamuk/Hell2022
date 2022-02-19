#include "Renderer.h"
#include "Renderer/TextBlitter.h"
#include "Renderer/NumberBlitter.h"
#include <random>
#include "Helpers/FileImporter.h"
#include "Helpers/AssetManager.h"

/*Shader Renderer::s_test_shader;
Shader Renderer::s_solid_color_shader;
Shader Renderer::s_textued_2D_quad_shader;
Shader Renderer::s_skinned_model_shader;
GBuffer Renderer::s_gBuffer;

// Pointers
PhysX* Renderer::p_physX; 
std::unordered_map<std::string, Model>* Renderer::p_models;
*/

void Renderer::Init(int screenWidth, int screenHeight)
{
    m_test_shader = Shader("test.vert", "test.frag");
    m_solid_color_shader = Shader("solidColor.vert", "solidColor.frag");
    m_textued_2D_quad_shader = Shader("textured2DquadShader.vert", "textured2DquadShader.frag");
    m_skinned_model_shader = Shader("skinnedmodel.vert", "skinnedmodel.frag");
    m_final_pass_shader = Shader("FinalPass.vert", "FinalPass.frag");

   // m_screenWidth = screenWidth;
   // m_screenHeight = screenHeight;

    s_gBuffer = GBuffer(screenWidth, screenHeight);

    glUniformBlockBinding(m_skinned_model_shader.ID, glGetUniformBlockIndex(m_skinned_model_shader.ID, "Matrices"), 0);
    glUniformBlockBinding(m_test_shader.ID, glGetUniformBlockIndex(m_test_shader.ID, "Matrices"), 0);

    glGenBuffers(1, &m_uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uboMatrices, 0, 2 * sizeof(glm::mat4));

}

/*void Renderer::SetModelPointers()
{
    m_modelPointers.p_CubeLines = AssetManager::GetModelPtr("CubeLines");
    m_modelPointers.p_Sphere = AssetManager::GetModelPtr("Sphere");
    m_modelPointers.p_SphereLines = AssetManager::GetModelPtr("SphereLines");
    m_modelPointers.p_UncappedCylinder = AssetManager::GetModelPtr("UncappedCylinder");
    m_modelPointers.p_HalfSphere2 = AssetManager::GetModelPtr("HalfSphere2");
    m_modelPointers.p_HalfSphere = AssetManager::GetModelPtr("HalfSphere");
    m_modelPointers.p_UncappedCylinderLines = AssetManager::GetModelPtr("UncappedCylinderLines");
    m_modelPointers.p_HalfSphereLines = AssetManager::GetModelPtr("HalfSphereLines");
    m_modelPointers.p_HalfSphereLines2 = AssetManager::GetModelPtr("HalfSphereLines2");
}*/


void Renderer::RenderFrame(Scene* scene, Camera* camera, int renderWidth, int renderHeight, int player, bool splitscreen)
{
    DrawGrid(camera);

    if (Input::s_showBulletDebug) {


        DrawPhysicsWorld(player);

        Shader* shader = &m_solid_color_shader;
        shader->use();

        // draw static entities with collision meshes
        for (auto entityStatic : scene->m_staticEntities)
        {
            if (entityStatic.m_model->m_hasCollisionMesh)
            {
                shader->setVec3("u_color", glm::vec3(0.4, 0.4, 0.4));
                entityStatic.DrawEntity(&m_solid_color_shader);
            }
        }
    }
    else
    {
        glEnable(GL_DEPTH_TEST);

        Shader* shader = &m_test_shader;
        shader->use();

        // Draw scene
        for (EntityStatic entityStatic : scene->m_staticEntities)
            entityStatic.DrawEntity(shader);




        glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO); 
        glBlendEquation(GL_FUNC_ADD);

        for (BloodPool bloodPool: scene->m_bloodPools)
            bloodPool.Draw(shader);
   
        glDisable(GL_BLEND);




        // Draw ragdolls
        shader = &m_skinned_model_shader;
        shader->use();
        shader->setBool("hasAnimation", true);
        shader->setMat4("model", glm::mat4(1));

        for (GameCharacter& gameCharacter : scene->m_gameCharacters)
        {
            gameCharacter.RenderSkinnedModel(shader);
        }

        if (player == 2) {
            p_player1->RenderCharacterModel(shader);
        }

        if (player == 1) {
            p_player2->RenderCharacterModel(shader);
        }


        // render player weapons
        if (player == 1) 
        {
            if (p_player1->m_isAlive) {
                p_player1->m_HUD_Weapon.Render(&m_skinned_model_shader);
                RenderPlayerCrosshair(renderWidth, renderHeight, p_player1);
            }
            else
            {

            }
        }
        if (player == 2) 
        {
            if (p_player2->m_isAlive)
            {
                p_player2->m_HUD_Weapon.Render(&m_skinned_model_shader);
                RenderPlayerCrosshair(renderWidth, renderHeight, p_player2);
            }
            else
            {
            }
        }
    }


    static unsigned int VAO = 0;
    if (VAO == 0) {
        unsigned int VBO;
        std::vector<glm::vec3> vertices;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        vertices.push_back(glm::vec3(0, 0, 0));
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    }

     if (player != 100) {
         glm::vec3 dotPos = p_player1->m_debugPos;// glm::vec3(1, 1, 0);
         Shader* shader = &m_solid_color_shader;
         shader->use();
         shader->setVec3("u_color", glm::vec3(1, 0, 1));
         shader->setVec3("u_cameraPos", dotPos);
         shader->setMat4("model", glm::mat4(1));
         glDisable(GL_DEPTH_TEST);
         glBindVertexArray(VAO);
         glPointSize(10);
         Transform trans;
         trans.position = dotPos;
         shader->setMat4("model", trans.to_mat4());
        // shader->setMat4("model", p_player2->m_debugMat);
        // glDrawArrays(GL_POINTS, 0, 1);
     }



     /*
     for (GameCharacter& gameCharacter : scene->m_gameCharacters)
     {
         // ragdoll joint dots
         static unsigned int VAO = 0;
         if (VAO == 0) {
             unsigned int VBO;
             std::vector<glm::vec3> vertices;
             glGenVertexArrays(1, &VAO);
             glGenBuffers(1, &VBO);
             vertices.push_back(glm::vec3(0, 0, 0));
             glBindVertexArray(VAO);
             glBindBuffer(GL_ARRAY_BUFFER, VBO);
             glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
             glEnableVertexAttribArray(0);
             glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
         }


         Shader* shader = &m_solid_color_shader;
         shader->use();
         shader->setVec3("u_color", glm::vec3(1, 0, 1));
         shader->setVec3("u_cameraPos", glm::vec3(0));
         shader->setMat4("model", glm::mat4(1));
         glDisable(GL_DEPTH_TEST);
         glBindVertexArray(VAO);
         glPointSize(10);

         for (RigidComponent& rigidComponent : gameCharacter.m_ragdoll.m_rigidComponents)
         {
             int index = gameCharacter.m_skinnedModel->m_BoneMapping[rigidComponent.correspondingJointName];
             glm::mat4 matrix;

             if (gameCharacter.m_skinningMethod == SkinningMethod::ANIMATED)
                 matrix = gameCharacter.m_transform.to_mat4() * gameCharacter.m_animatedDebugTransforms_Animated[index];

             shader->setMat4("model", matrix);
             glDrawArrays(GL_POINTS, 0, 1);
         }
     }*/


    glDisable(GL_DEPTH_TEST);
    glDrawBuffer(GL_COLOR_ATTACHMENT1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_gBuffer.gAlbedo);

    if (player == 1) {
        m_final_pass_shader.use();
        m_final_pass_shader.setFloat("u_timeSinceDeath", p_player1->m_timeSinceDeath);
        if (splitscreen)
            Draw_2_Player_Split_Screen_Quad_Top(&m_final_pass_shader);
        else
            DrawFullScreenQuad(&m_final_pass_shader);

        if (!p_player1->m_isAlive && p_player1->m_timeSinceDeath > 3.125)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            glDisable(GL_DEPTH_TEST);
            float scale = 1.5f;
            float width = (1.0f / 1920) * AssetManager::GetTexturePtr("PressStart")->m_width * scale;
            float height = (1.0f / 1080) * AssetManager::GetTexturePtr("PressStart")->m_height * scale * (CoreGL::s_currentHeight / renderHeight);
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(width, height, 1));
            DrawQuad(&m_textued_2D_quad_shader, modelMatrix, AssetManager::GetTexturePtr("PressStart")->ID);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }
    }
    if (player == 2) {
        m_final_pass_shader.use();
        m_final_pass_shader.setFloat("u_timeSinceDeath", p_player2->m_timeSinceDeath);
        Draw_2_Player_Split_Screen_Quad_Bottom(&m_final_pass_shader);
    
        
        if (!p_player2->m_isAlive && p_player2->m_timeSinceDeath > 3.125)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
            glDisable(GL_DEPTH_TEST);
            float scale = 1.5f;
            float width = (1.0f / 1920) * AssetManager::GetTexturePtr("PressStart")->m_width * scale;
            float height = (1.0f / 1080) * AssetManager::GetTexturePtr("PressStart")->m_height * scale * (CoreGL::s_currentHeight / renderHeight);
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(width, height, 1));
            DrawQuad(&m_textued_2D_quad_shader, modelMatrix, AssetManager::GetTexturePtr("PressStart")->ID);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }    
    }




    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

void Renderer::RenderPlayerCrosshair(int renderWidth, int renderHeight, Player* player)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glDisable(GL_DEPTH_TEST);

    float crosshairSize = 16;
    float width = (1.0f / 1920) * crosshairSize;
    float height = (1.0f / 1080) * crosshairSize * (CoreGL::s_currentHeight / renderHeight);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(width, height, 1)); 
    DrawQuad(&m_textued_2D_quad_shader, modelMatrix, AssetManager::GetTexturePtr("CrosshairCross")->ID);


    

    width = (1.0f / 1920) * AssetManager::GetTexturePtr("HUDGradient")->m_width;
    height = (1.0f / 1080) * AssetManager::GetTexturePtr("HUDGradient")->m_height * (CoreGL::s_currentHeight / renderHeight);
    glm::vec3 position = glm::vec3(0.8f, -0.7785f, 0);
    modelMatrix = glm::translate(glm::mat4(1), position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(width, height, 1));
   // DrawQuad(&m_textued_2D_quad_shader, modelMatrix, AssetManager::GetTexturePtr("HUDGradient")->ID);
    




    
   
    // Ammo
    //Quad2D::RenderQuad(shader, AssetManager::GetTextureByName("HUDGradient"), 1604, 934);

    glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("NumSheet")->ID);

    std::string ammo_in_clip = std::to_string(player->m_ammo_in_clip);
    std::string m_ammo_total = std::to_string(player->m_ammo_total);
    glm::vec3 ammoColor = glm::vec3(0.26f, 0.78f, 0.33f);

    char* cstr = new char[ammo_in_clip.length() + 1];
    strcpy(cstr, ammo_in_clip.c_str());
    char* cstr2 = new char[m_ammo_total.length() + 1];
    strcpy(cstr2, m_ammo_total.c_str());
    const char* slash = "/";
    NumberBlitter::DrawTextBlit(&m_textued_2D_quad_shader, slash, 1700, 943, renderWidth, renderHeight);
    NumberBlitter::DrawTextBlit(&m_textued_2D_quad_shader, cstr2, 1715, 943, renderWidth, renderHeight, 0.8);
    NumberBlitter::DrawTextBlit(&m_textued_2D_quad_shader, cstr, 1695, 943, renderWidth, renderHeight, 1.0f, ammoColor, false);
    m_textued_2D_quad_shader.setVec3("colorTint", glm::vec3(1, 1, 1));


  /*  shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);


    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    */

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::DrawGrid(Camera* camera)
{
    static unsigned int VAO = 0;
    static unsigned int vertCount;
    int grid_size = 10;
    if (VAO == 0) {
        unsigned int VBO;
        std::vector<glm::vec3> vertices;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        float y = -0.05;
        for (int x = -grid_size / 2; x < grid_size / 2 + 1; x++) {
            for (int z = -grid_size / 2; z < grid_size / 2 + 1; z++) {
                vertices.push_back(glm::vec3(x * EDITOR_GRID_SIZE, y, -grid_size / 2 * EDITOR_GRID_SIZE));
                vertices.push_back(glm::vec3(x * EDITOR_GRID_SIZE, y, grid_size / 2 * EDITOR_GRID_SIZE));
                vertices.push_back(glm::vec3(-grid_size / 2 * EDITOR_GRID_SIZE, y, z * EDITOR_GRID_SIZE));
                vertices.push_back(glm::vec3(grid_size / 2 * EDITOR_GRID_SIZE, y, z * EDITOR_GRID_SIZE));
            }
        }
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        vertCount = vertices.size();
        //std::cout << "vertCount: " << vertCount << "\n";
    }

    Shader* shader = &m_solid_color_shader;
    shader->use();
    shader->setVec3("u_color", glm::vec3(0.509, 0.333, 0.490));
    shader->setVec3("u_cameraPos", camera->m_transform.position);
    
    int grid_repeat = 100;
    for (int x = -grid_repeat / 2; x < grid_repeat / 2 + 1; x++) {
        for (int z = -grid_repeat / 2; z < grid_repeat / 2 + 1; z++) {
            Transform trans;
            trans.position.x = x * grid_size * EDITOR_GRID_SIZE;
            trans.position.z = z * grid_size * EDITOR_GRID_SIZE;
            shader->setMat4("model", trans.to_mat4());
            glBindVertexArray(VAO);
            glDrawArrays(GL_LINES, 0, vertCount);
        }
    }
}

void Renderer::TextBlitterPass(Shader* shader)
{
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("CharSheet")->ID);
    TextBlitter::BlitText(TextBlitter::s_textToBlit, false); // this congfigures internal blitter variables
    TextBlitter::DrawTextBlit(shader);                       // this draws the thing
    TextBlitter::ResetBlitter();                             // resets internal variables
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::DrawLine(Shader* shader, Line line, glm::mat4 modelMatrix = glm::mat4(1))
{
    static unsigned int VAO = 0;
    static unsigned int VBO;
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }
    float vertices[] = {
        line.start_pos.r,  line.start_pos.g,  line.start_pos.b,  line.start_color.r,  line.start_color.g,  line.start_color.b,
        line.end_pos.r,  line.end_pos.g,  line.end_pos.b,  line.end_color.r,  line.end_color.g,  line.end_color.b,
    };
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    shader->setMat4("model", modelMatrix);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 6);
}

void Renderer::HotLoadShaders()
{
    m_test_shader.ReloadShader();
    m_solid_color_shader.ReloadShader();
    m_textued_2D_quad_shader.ReloadShader();
    m_skinned_model_shader.ReloadShader();
    m_final_pass_shader.ReloadShader();
}

void Renderer::DrawPoint(Shader* shader, glm::vec3 position, glm::vec3 color)
{
    static unsigned int VAO = 0;
    static unsigned int VBO;
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    float vertices[] = {
        position.x, position.y, position.z, color.r, color.g, color.b,
    };
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glm::mat4 modelMatrix = glm::mat4(1);
    shader->setMat4("model", modelMatrix);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, 1);
}

void Renderer::RenderFinalImage()
{
 /*   Shader* shader = &m_textued_2D_quad_shader;
    shader->use();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_gBuffer.gAlbedo);
    //glViewport(0, 0, m_screenWidth, m_screenHeight);

    static GLuint VAO = 0;
    if (VAO == 0)
    {
        float quadVertices[] = {
            // positions         texcoords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
                1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    shader->setMat4("model", glm::mat4(1));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);*/
}


void Renderer::DrawPhysicsWorld(int player)
{
    bool drawSolids = true;

    int primitiveType = GL_TRIANGLES;

    glEnable(GL_DEPTH_TEST);

    Shader* shader = &m_solid_color_shader;
    shader->use();

   // for (Entity )

    PxScene* scene;
    PxGetPhysics().getScenes(&scene, 1);
    PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
    if (nbActors)
    {
        std::vector<PxRigidActor*> actors(nbActors);
        scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);

        PxShape* shapes[128];
        int numActors = static_cast<PxU32>(actors.size());

        for (PxU32 i = 0; i < numActors; i++)
        {
            // skip this shape if part of the current players ragdoll
            void* parent = actors[i]->userData;
            if (player == 1 && parent == p_player1)
                continue;
            if (player == 2 && parent == p_player2)
                continue;


            const PxU32 nbShapes = actors[i]->getNbShapes();

            PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
            actors[i]->getShapes(shapes, nbShapes);
            const bool sleeping = actors[i]->is<PxRigidDynamic>() ? actors[i]->is<PxRigidDynamic>()->isSleeping() : false;

            for (PxU32 j = 0; j < nbShapes; j++)
            {
                const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
                const PxGeometryHolder h = shapes[j]->getGeometry();
                const PxGeometry geom = h.any();

                auto shape = shapes[j]->getGeometry();

                auto PxMatrix = shapePose;

                glm::mat4 matrix;
                for (int x = 0; x < 4; x++)
                    for (int y = 0; y < 4; y++)
                        matrix[x][y] = PxMatrix[x][y];




                //auto models = *p_models;

                if (geom.getType() == PxGeometryType::eBOX) {

                    Transform transform;
                    transform.scale = glm::vec3(shape.box().halfExtents.x * 2, shape.box().halfExtents.y * 2, shape.box().halfExtents.z * 2);

                    glDepthFunc(GL_LEQUAL);
                    shader->setVec3("u_color", glm::vec3(1, 1, 1));
                    AssetManager::GetModelPtr("CubeLines")->Draw(shader, matrix * transform.to_mat4(), GL_LINES);

                    if (drawSolids) {
                        shader->setVec3("u_color", glm::vec3(0.4, 0.4, 0.4));
                        AssetManager::GetModelPtr("Cube")->Draw(shader, matrix * transform.to_mat4(), GL_TRIANGLES);
                    }




                    shader->setVec3("u_color", glm::vec3(0.4, 0.4, 0.4));
                }
                if (shape.any().getType() == PxGeometryType::eSPHERE)
                {
                    Transform transform;
                    transform.scale = glm::vec3(shape.sphere().radius);

                    if (drawSolids) {
                        shader->setVec3("u_color", glm::vec3(0.4, 0.4, 0.4));
                        AssetManager::GetModelPtr("Sphere")->Draw(shader, matrix * transform.to_mat4(), GL_TRIANGLES);
                    }

                    shader->setVec3("u_color", glm::vec3(1, 1, 1));
                    AssetManager::GetModelPtr("SphereLines")->Draw(shader, matrix * transform.to_mat4(), GL_LINES);
                }

                if (geom.getType() == PxGeometryType::eCAPSULE)
                {
                    float halfExtent = shape.capsule().halfHeight;
                    float radius = shape.capsule().radius;

                    Transform cylinderTransform;
                    cylinderTransform.scale.x = halfExtent * 2;
                    cylinderTransform.scale.y = radius;
                    cylinderTransform.scale.z = radius;
                    Transform topSphereTransform;
                    topSphereTransform.position.x += halfExtent;
                    topSphereTransform.scale = glm::vec3(radius);
                    Transform bottomSphereTransform;
                    bottomSphereTransform.position.x -= halfExtent;
                    bottomSphereTransform.scale = glm::vec3(radius);

                    if (drawSolids) {
                        shader->setVec3("u_color", glm::vec3(0.4, 0.4, 0.4));
                        AssetManager::GetModelPtr("UncappedCylinder")->Draw(shader, matrix * cylinderTransform.to_mat4(), GL_TRIANGLES);
                        AssetManager::GetModelPtr("HalfSphere2")->Draw(shader, matrix * bottomSphereTransform.to_mat4(), GL_TRIANGLES);
                        AssetManager::GetModelPtr("HalfSphere")->Draw(shader, matrix * topSphereTransform.to_mat4(), GL_TRIANGLES);
                    }

                    shader->setVec3("u_color", glm::vec3(1, 1, 1));
                    AssetManager::GetModelPtr("UncappedCylinderLines")->Draw(shader, matrix * cylinderTransform.to_mat4(), GL_LINES);
                    AssetManager::GetModelPtr("HalfSphereLines")->Draw(shader, matrix * topSphereTransform.to_mat4(), GL_LINES);
                    AssetManager::GetModelPtr("HalfSphereLines2")->Draw(shader, matrix * bottomSphereTransform.to_mat4(), GL_LINES);
                }

            }
        }
    }
}

void Renderer::ReconfigureFrameBuffers(int height, int width)
{
    s_gBuffer.Configure(height, width);
}

void Renderer::DrawFullScreenQuad(Shader* shader)
{
    static GLuint VAO = 0;
    if (VAO == 0)
    {
        float quadVertices[] = {
            // positions         texcoords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
                1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }	
    shader->use();
    shader->setMat4("model", glm::mat4(1));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::Draw_2_Player_Split_Screen_Quad_Top(Shader* shader)
{
    static GLuint VAO = 0;
    if (VAO == 0)
    {
        float quadVertices[] = {
            // positions         texcoords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.5f,
                1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
                1.0f, -1.0f, 0.0f,  1.0f, 0.5f,
        };
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    shader->use();
    shader->setMat4("model", glm::mat4(1));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void Renderer::Draw_2_Player_Split_Screen_Quad_Bottom(Shader* shader)
{
    static GLuint VAO = 0;
    if (VAO == 0)
    {
        float quadVertices[] = {
            // positions         texcoords
            -1.0f,  1.0f, 0.0f,  0.0f, 0.5f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                1.0f,  1.0f, 0.0f,  1.0f, 0.5f,
                1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    shader->use();
    shader->setMat4("model", glm::mat4(1));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::ClearFBOs(int screenWidth, int screenHeight)
{
    glBindFramebuffer(GL_FRAMEBUFFER, s_gBuffer.ID);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearColor(0.2f, 0.2f, 0.2f, 0.2f);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void Renderer::DrawQuad(Shader* shader, glm::mat4 modelMatrix, GLuint texID)
{
    static unsigned int VAO = 0;
    if (VAO == 0)
    {
        float quadVertices[] = {
            // positions         texcoords
            -1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        };
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    shader->use();
    shader->setMat4("model", modelMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
