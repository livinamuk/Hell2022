#include "Renderer.h"
#include "Renderer/TextBlitter.h"
#include "Renderer/NumberBlitter.h"
#include <random>
#include "Helpers/FileImporter.h"
#include "Helpers/AssetManager.h"
#include <chrono>
#include <thread>
#include "Core/GameData.h"

Shader Renderer::s_geometry_shader;
Shader Renderer::s_solid_color_shader;
Shader Renderer::s_textued_2D_quad_shader;
Shader Renderer::s_lighting_shader;
Shader Renderer::s_final_pass_shader;
Shader Renderer::s_solid_color_shader_editor; 
Shader Renderer::s_shadow_map_shader;
Shader Renderer::s_skybox_shader;
GBuffer Renderer::s_gBuffer;
std::vector<BlurBuffer> Renderer::s_BlurBuffers_p1;
std::vector<BlurBuffer> Renderer::s_BlurBuffers_p2;
unsigned int Renderer::m_uboMatrices;
unsigned int Renderer::m_uboLights;
bool Renderer::s_showBuffers = false;
GLuint Renderer::s_centeredQuadVAO;
GLuint Renderer::s_quadVAO;
GLuint Renderer::s_brdfLUTTextureID;
Shader Renderer::s_brdf_shader;	
Shader Renderer::s_env_map_shader;
Shader Renderer::s_SH_shader; 
Shader Renderer::s_animated_quad_shader;
Shader Renderer::s_postProcessingShader;
Shader Renderer::s_decal_shader;
Shader Renderer::s_blood_decal_shader;
Shader Renderer::s_horizontal_blur_shader;
Shader Renderer::s_vertical_blur_shader;
Shader Renderer::s_volumetric_blood_shader;
Shader Renderer::s_instanced_geometry_shader;
bool Renderer::s_showBulletDecalPositions = false;

MuzzleFlash Renderer::s_muzzleFlash;
bool Renderer::m_firstRenderLoop = true;

// debug
bool renderPlayerWeaponsAlways = false;// true;
bool renderPlayerCharacterModels = true;// false;// true;


void Renderer::Init(int screenWidth, int screenHeight)
{
	s_geometry_shader = Shader("geometry.vert", "geometry.frag");
    s_instanced_geometry_shader = Shader("geometryInstanced.vert", "geometry.frag");
    s_solid_color_shader = Shader("solidColor.vert", "solidColor.frag");
    s_solid_color_shader_editor = Shader("solidColorEditor.vert", "solidColorEditor.frag");
    s_textued_2D_quad_shader = Shader("textured2DquadShader.vert", "textured2DquadShader.frag");
    s_lighting_shader = Shader("lighting.vert", "lighting.frag");
    s_final_pass_shader = Shader("FinalPass.vert", "FinalPass.frag");
    s_shadow_map_shader = Shader("shadowmap.vert", "shadowmap.frag", "shadowmap.geom");
    s_skybox_shader = Shader("skybox.vert", "skybox.frag");
    s_brdf_shader = Shader("brdf.vert", "brdf.frag");
    s_env_map_shader = Shader("envMap.vert", "envMap.frag", "envMap.geom");
    s_SH_shader = Shader("SH.vert", "SH.frag");
	s_animated_quad_shader = Shader("animatedQuad.vert", "animatedQuad.frag");
	s_postProcessingShader = Shader("PostProcessing.vert", "PostProcessing.frag");
	s_decal_shader = Shader("decals.vert", "decals.frag");
	s_blood_decal_shader = Shader("blood_decal_shader.vert", "blood_decal_shader.frag");
	s_horizontal_blur_shader = Shader("blurHorizontal.vert", "blur.frag");
	s_vertical_blur_shader = Shader("blurVertical.vert", "blur.frag");
    s_volumetric_blood_shader = Shader("bloodVolumetric.vert", "bloodVolumetric.frag");
    
    s_gBuffer = GBuffer(screenWidth, screenHeight);

    // Create the blur buffers FBOs
    // width and height of 1 cause you are only making the FBO ID and shit here
    // the real sizes are made in ReconfigureFrameBuffers()
	s_BlurBuffers_p1.clear();
	s_BlurBuffers_p2.clear();
    for (int i = 0; i < 4; i++) {
		s_BlurBuffers_p1.push_back(BlurBuffer(1, 1));
		s_BlurBuffers_p2.push_back(BlurBuffer(1, 1));
    }
    ReconfigureFrameBuffers(screenWidth, screenHeight);

    float quadVertices[] = {
        // positions         texcoords
        0.0f, 1.0f,  0.0f, 1.0f,
        0.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
    };
    GLuint VBO = 0;
    glGenVertexArrays(1, &s_quadVAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(s_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex2D), &quadVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    float centeredQuadVertices[] = {
        // positions         texcoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
    };
    GLuint VBO2 = 0;
    glGenVertexArrays(1, &s_centeredQuadVAO);
    glGenBuffers(1, &VBO2);
    glBindVertexArray(s_centeredQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex2D), &centeredQuadVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Matices UBO
    glGenBuffers(1, &m_uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 4, NULL, GL_STATIC_DRAW);       // 4 matrices
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_uboMatrices, 0, sizeof(glm::mat4) * 4);   // 4 matrices

    // Lights UBO
    glGenBuffers(1, &m_uboLights);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboLights);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LightUniformBlock) * MAX_LIGHTS, NULL, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_uboLights, 0, sizeof(LightUniformBlock) * MAX_LIGHTS);

    CreateBRDFLut();

    s_muzzleFlash.Init();
}

void Renderer::DrawTangentDebugAxis(Shader* shader, glm::mat4 modelMatrix, float lineLength)
{
    static unsigned int VAO = 0;
    if (VAO == 0)
    {
        unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glm::vec3 vertices[12];
        vertices[0] = (glm::vec3(0, 0, 0)); // origin
        vertices[1] = (glm::vec3(1, 0, 0)); // red
        vertices[2] = (glm::vec3(lineLength, 0, 0)); // X Axis
        vertices[3] = (glm::vec3(1, 0, 0)); // red
        vertices[4] = (glm::vec3(0, 0, 0)); // origin
        vertices[5] = (glm::vec3(0, 1, 0)); // green
        vertices[6] = (glm::vec3(0, lineLength, 0)); // Y Axis
        vertices[7] = (glm::vec3(0, 1, 0)); // green
        vertices[8] = (glm::vec3(0, 0, 0)); // origin
        vertices[9] = (glm::vec3(0, 0, 1)); // green
        vertices[10] = (glm::vec3(0, 0, lineLength)); // Z Axis
        vertices[11] = (glm::vec3(0, 0, 1)); // green
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(VAO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    shader->setMat4("model", modelMatrix);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 6);
}

void Renderer::RenderFrame(Camera* camera, int renderWidth, int renderHeight, int player)
{
    // calculate projection matrix (requrired for ADS scope)
    Player* p = GetPlayerFromIndex(player);
    p->m_camera.CalculateProjectionMatrix(renderWidth, renderHeight);

    if (Input::s_showBulletDebug) {

        DrawPhysicsWorld(player);

        Shader* shader = &s_solid_color_shader;
        shader->use();

        // draw static entities with collision meshes
        for (auto entityStatic : GameData::s_staticEntities)
        {
            if (entityStatic.m_model->m_hasCollisionMesh)
            {
                shader->setVec3("u_color", glm::vec3(0.4, 0.4, 0.4));
                entityStatic.DrawEntity(&s_solid_color_shader);
            }
        }
    }
    else
    {
        if (player == 1) {

            if (GameData::s_renderEnvMaps) {
                GameData::s_renderEnvMaps = false;
                EnvMapPass();
            }

            ShadowMapPass();
            IndirectShadowMapPass();
        }

        GeometryPass(player, renderWidth, renderHeight);
        DecalPass(player, renderWidth, renderHeight);
        VolumetricBloodPass(player, renderWidth, renderHeight);
        LightingPass(player, renderWidth, renderHeight);
        MuzzleFlashPass(player, renderWidth, renderHeight);

        RenderDebugShit();

        PostProcessingPass(player); 
        EmissiveBlurPass(player, renderWidth, renderHeight, 4);


     //   glDrawBuffer(GL_COLOR_ATTACHMENT3);


    }





	SetViewport(player);
	glBindFramebuffer(GL_FRAMEBUFFER, s_gBuffer.ID);

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
         glm::vec3 dotPos = GameData::s_player1.m_debugPos;// glm::vec3(1, 1, 0);
         Shader* shader = &s_solid_color_shader;
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
    glDrawBuffer(GL_COLOR_ATTACHMENT3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_gBuffer.gPostProcessed);
   /* glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, s_gBuffer.gPostProcessed);
    glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, s_gBuffer.gNormal);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, s_gBuffer.gEmissive);*/
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, s_gBuffer.gRMA);

    int CHROM_ABZ_mode = 0;

    if (GameData::s_splitScreen && player == 1)
		CHROM_ABZ_mode = 1;
	else if (GameData::s_splitScreen && player == 2)
		CHROM_ABZ_mode = 2;

    s_final_pass_shader.use();
    s_final_pass_shader.setInt("u_CHROM_ABZ_mode", CHROM_ABZ_mode);

    s_final_pass_shader.setFloat("u_time", CoreGL::GetGLTime());

    if (player == 1) {
        // light bulbs
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, s_BlurBuffers_p1[0].textureA);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, s_BlurBuffers_p1[1].textureA);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, s_BlurBuffers_p1[2].textureA);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, s_BlurBuffers_p1[3].textureA);

        s_final_pass_shader.setFloat("u_timeSinceDeath", GameData::s_player1.m_timeSinceDeath);

        DrawViewportQuad(&s_final_pass_shader, GetViewportSize(player));

        if (!GameData::s_player1.m_isAlive && GameData::s_player1.m_timeSinceDeath > 3.125)
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
            DrawQuad(&s_textued_2D_quad_shader, modelMatrix, AssetManager::GetTexturePtr("PressStart")->ID);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }
    }
    if (player == 2) {
		// light bulbs
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, s_BlurBuffers_p2[0].textureA);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, s_BlurBuffers_p2[1].textureA);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, s_BlurBuffers_p2[2].textureA);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, s_BlurBuffers_p2[3].textureA);
        s_final_pass_shader.use();
        s_final_pass_shader.setFloat("u_timeSinceDeath", GameData::s_player2.m_timeSinceDeath);
        //Draw_2_Player_Split_Screen_Quad_Bottom(&s_final_pass_shader);
        DrawViewportQuad(&s_final_pass_shader, GetViewportSize(player));
        //DrawViewportQuad(&s_final_pass_shader, ViewportSize::SPLITSCREEN_BOTTOM);
    
        
        if (!GameData::s_player2.m_isAlive && GameData::s_player2.m_timeSinceDeath > 3.125)
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
            DrawQuad(&s_textued_2D_quad_shader, modelMatrix, AssetManager::GetTexturePtr("PressStart")->ID);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }    
    }


    // render player weapons
    if (player == 1)
    {
        if (GameData::s_player1.m_isAlive)
            RenderPlayerCrosshair(renderWidth, renderHeight, &GameData::s_player1);
    }
    if (player == 2)
    {
        if (GameData::s_player2.m_isAlive)
            RenderPlayerCrosshair(renderWidth, renderHeight, &GameData::s_player2);
    }

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    m_firstRenderLoop = false;
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
    DrawQuad(&s_textued_2D_quad_shader, modelMatrix, AssetManager::GetTexturePtr("CrosshairCross")->ID);




    width = (1.0f / 1920) * AssetManager::GetTexturePtr("HUDGradient")->m_width;
    height = (1.0f / 1080) * AssetManager::GetTexturePtr("HUDGradient")->m_height * (CoreGL::s_currentHeight / renderHeight);
    glm::vec3 position = glm::vec3(0.8f, -0.7785f, 0);
    modelMatrix = glm::translate(glm::mat4(1), position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(width, height, 1));
    // DrawQuad(&m_textued_2D_quad_shader, modelMatrix, AssetManager::GetTexturePtr("HUDGradient")->ID);







     // Ammo
     //Quad2D::RenderQuad(shader, AssetManager::GetTextureByName("HUDGradient"), 1604, 934);

    if (player->m_gun != Player::Gun::KNIFE)
    {
        glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("NumSheet")->ID);

        std::string ammo_in_clip = std::to_string(player->GetCurrentGunAmmoInClip());
        std::string m_ammo_total = std::to_string(player->GetCurrentGunTotalAmmo());
        glm::vec3 ammoColor = glm::vec3(0.26f, 0.78f, 0.33f);

        char* cstr = new char[ammo_in_clip.length() + 1];
        strcpy(cstr, ammo_in_clip.c_str());
        char* cstr2 = new char[m_ammo_total.length() + 1];
        strcpy(cstr2, m_ammo_total.c_str());
        const char* slash = "/";
        NumberBlitter::DrawTextBlit(&s_textued_2D_quad_shader, slash, 1700, 943, renderWidth, renderHeight);
        NumberBlitter::DrawTextBlit(&s_textued_2D_quad_shader, cstr2, 1715, 943, renderWidth, renderHeight, 0.8);
        NumberBlitter::DrawTextBlit(&s_textued_2D_quad_shader, cstr, 1695, 943, renderWidth, renderHeight, 1.0f, ammoColor, false);
        s_textued_2D_quad_shader.setVec3("colorTint", glm::vec3(1, 1, 1));
    }

  /*  shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);


    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    */

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Renderer::DrawGrid(glm::vec3 cameraPos, bool renderFog)
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

    Shader* shader = &s_solid_color_shader;
    shader->use();
    shader->setVec3("u_color", glm::vec3(0.509, 0.333, 0.490));

    shader->setVec3("u_cameraPos", cameraPos);
    shader->setBool("u_renderFog", renderFog);
    
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

    shader->setBool("u_renderFog", false);
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

void Renderer::DrawLine(Shader* shader, glm::vec3 p1, glm::vec3 p2, glm::vec3 color)
{
    static unsigned int VAO = 0;
    static unsigned int VBO;
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }
    float vertices[] = { p1.x, p1.y, p1.z, p2.x, p2.y, p2.z };
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    shader->setMat4("model", glm::mat4(1));
    shader->setVec3("u_color", color);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);

    //glDeleteBuffers(1, &VAO);
}

void Renderer::HotLoadShaders()
{
    s_geometry_shader.ReloadShader();
    s_instanced_geometry_shader.ReloadShader();
    s_solid_color_shader.ReloadShader();
    s_solid_color_shader_editor.ReloadShader();
    s_textued_2D_quad_shader.ReloadShader();
    s_lighting_shader.ReloadShader();
    s_final_pass_shader.ReloadShader();
    s_shadow_map_shader.ReloadShader();
    s_skybox_shader.ReloadShader();
    s_env_map_shader.ReloadShader();
    s_animated_quad_shader.ReloadShader();
    s_postProcessingShader.ReloadShader();
    s_decal_shader.ReloadShader();
    s_blood_decal_shader.ReloadShader();
	s_horizontal_blur_shader.ReloadShader();
	s_vertical_blur_shader.ReloadShader();
    s_volumetric_blood_shader.ReloadShader();
}

void Renderer::DrawPoint(Shader* shader, glm::vec3 position, glm::vec3 color)
{
    Transform trans;
      trans.position = position;
    DrawPoint(shader, trans.to_mat4(), color);
}

void Renderer::DrawPoint(Shader* shader, glm::mat4 modelMatrix, glm::vec3 color)
{
    static unsigned int VAO = 0;
    if (VAO == 0)
    {
        static unsigned int VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        float vertices[] = { 0, 0, 0 };
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(VAO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glm::mat4 modelMatrix = glm::mat4(1);
    }

    shader->setMat4("model", modelMatrix);
    shader->setVec3("u_color", color);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, 1);
}

void Renderer::DrawPhysicsWorld(int player)
{
    bool drawSolids = true;

    int primitiveType = GL_TRIANGLES;

    glEnable(GL_DEPTH_TEST);

    Shader* shader = &s_solid_color_shader;
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
            if (player == 1 && parent == &GameData::s_player1)
                continue;
            if (player == 2 && parent == &GameData::s_player2)
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

void Renderer::ReconfigureFrameBuffers(int width, int height)
{
	s_gBuffer.Configure(width, height);

    // If fullscreen, you only need to do p1 and its just half the screen size
	if (!GameData::s_splitScreen) {
		s_BlurBuffers_p1[0].Configure(width / 2, height / 2);
		s_BlurBuffers_p1[1].Configure(width / 4, height / 4);
		s_BlurBuffers_p1[2].Configure(width / 8, height / 8);
		s_BlurBuffers_p1[3].Configure(width / 16, height / 16);
		s_BlurBuffers_p2[0].Configure(0, 1);
		s_BlurBuffers_p2[1].Configure(0, 1);
		s_BlurBuffers_p2[2].Configure(0, 1);
		s_BlurBuffers_p2[3].Configure(0, 1);
	}    
    // If split screen then heights are divded by 2
	else {
		s_BlurBuffers_p1[0].Configure(width / 2, height / 2 / 2);
		s_BlurBuffers_p1[1].Configure(width / 4, height / 4 / 2);
		s_BlurBuffers_p1[2].Configure(width / 8, height / 8 / 2);
		s_BlurBuffers_p1[3].Configure(width / 16, height / 16 / 2);
		s_BlurBuffers_p2[0].Configure(width / 2, height / 2 / 2);
		s_BlurBuffers_p2[1].Configure(width / 4, height / 4 / 2);
		s_BlurBuffers_p2[2].Configure(width / 8, height / 8 / 2);
		s_BlurBuffers_p2[3].Configure(width / 16, height / 16 / 2);
	}
}



void Renderer::DrawViewportQuad(Shader* shader, ViewportSize viewportSize)
{
    static GLuint VAO_fullscreen = 0;
    static GLuint VAO_splitscreen_top;
    static GLuint VAO_splitscreen_bottom;
    
    if (VAO_fullscreen == 0) {
        float quadVertices[] = {
            // positions         texcoords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
                1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        unsigned int VBO;
        glGenVertexArrays(1, &VAO_fullscreen);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO_fullscreen);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    } 

    if (VAO_splitscreen_top == 0) {
        float quadVertices[] = {
            // positions         texcoords
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.5f,
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.5f,
        };
        unsigned int VBO;
        glGenVertexArrays(1, &VAO_splitscreen_top);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO_splitscreen_top);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    if (VAO_splitscreen_bottom == 0) {
        float quadVertices[] = {
            // positions         texcoords
            -1.0f,  1.0f, 0.0f,  0.0f, 0.5f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 0.5f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        unsigned int VBO;
        glGenVertexArrays(1, &VAO_splitscreen_bottom);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO_splitscreen_bottom);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    shader->use();
    shader->setMat4("model", glm::mat4(1));

    if (viewportSize == ViewportSize::FULL_SCREEN)
        glBindVertexArray(VAO_fullscreen);
    else if (viewportSize == ViewportSize::SPLITSCREEN_TOP)
        glBindVertexArray(VAO_splitscreen_top);
    else if (viewportSize == ViewportSize::SPLITSCREEN_BOTTOM)
        glBindVertexArray(VAO_splitscreen_bottom);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::SetViewport(int player)
{
    if (!GameData::s_splitScreen)
        glViewport(0, 0, CoreGL::s_currentWidth, CoreGL::s_currentHeight);

	else if (GameData::s_playerCount == 2)
	{
		if (player == 1)
			glViewport(0, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth, CoreGL::s_currentHeight / 2);
		else if (player == 2)
			glViewport(0, 0, CoreGL::s_currentWidth, CoreGL::s_currentHeight / 2);
	}
	else if (GameData::s_playerCount > 2)
	{
		if (player == 1)
			glViewport(0, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		else if (player == 2)
			glViewport(0, 0, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		else if (player == 3)
			glViewport(CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
		else if (player == 4)
			glViewport(CoreGL::s_currentWidth / 2, CoreGL::s_currentWidth / 2, CoreGL::s_currentWidth / 2, CoreGL::s_currentHeight / 2);
	}
}

float Renderer::GetViewportWidth(int player)
{
    if (!GameData::s_splitScreen || GameData::s_playerCount == 2)
        return CoreGL::s_currentWidth;
    else
        return CoreGL::s_currentWidth / 2;
}

float Renderer::GetViewportHeight(int player)
{
    if (!GameData::s_splitScreen)
        return CoreGL::s_currentHeight;
    else
        return CoreGL::s_currentHeight / 2;
}

Renderer::ViewportSize Renderer::GetViewportSize(int player)
{
    if (!GameData::s_splitScreen)
        return ViewportSize::FULL_SCREEN;

    else if (GameData::s_playerCount == 2) 
    {
        if (player == 1)
            return ViewportSize::SPLITSCREEN_TOP;
        else if (player == 2)
            return ViewportSize::SPLITSCREEN_BOTTOM;
	}
	else if (GameData::s_playerCount > 2)
	{
		if (player == 1)
			return ViewportSize::QUAD_SPLITSCREEN_TOP_LEFT;
		else if (player == 2)
			return ViewportSize::QUAD_SPLITSCREEN_TOP_RIGHT;
		else if (player == 3)
			return ViewportSize::QUAD_SPLITSCREEN_BOTTOM_LEFT;
		else if (player == 4)
			return ViewportSize::QUAD_SPLITSCREEN_BOTTOM_RIGHT;
	}
    // Fall back. But if you get here then something is wrong
    std::cout << "something is wrong\n";
	return ViewportSize::FULL_SCREEN;
}

/*void Renderer::DrawFullScreenQuad(Shader * shader)
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
}*/

/*void Renderer::Draw_2_Player_Split_Screen_Quad_Top(Shader* shader)
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
*/
/*
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
}*/

void Renderer::ClearFBOs(int screenWidth, int screenHeight)
{
    glBindFramebuffer(GL_FRAMEBUFFER, s_gBuffer.ID);

    unsigned int attachments[7] = { 
        GL_COLOR_ATTACHMENT0, 
        GL_COLOR_ATTACHMENT1, 
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6 };

    glDrawBuffers(5, attachments);

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

void Renderer::CreateBRDFLut()
{
    unsigned int BRDFLut_Fbo;
    unsigned int BRDFLut_Rbo;
    glGenFramebuffers(1, &BRDFLut_Fbo);
    glGenRenderbuffers(1, &BRDFLut_Rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, BRDFLut_Fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, BRDFLut_Rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, BRDFLut_Rbo);
    glGenTextures(1, &s_brdfLUTTextureID);
    glBindTexture(GL_TEXTURE_2D, s_brdfLUTTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, BRDFLut_Fbo);
    glBindRenderbuffer(GL_RENDERBUFFER, BRDFLut_Rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_brdfLUTTextureID, 0);
    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    s_brdf_shader.use();
    s_brdf_shader.setMat4("model", glm::mat4(1));
    glBindVertexArray(s_centeredQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glDeleteBuffers(1, &BRDFLut_Fbo);
    glDeleteRenderbuffers(1, &BRDFLut_Rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderDebugShit()
{
 //   return;
    glDisable(GL_DEPTH_TEST);





    Shader* shader = &s_solid_color_shader;
    shader->use();

    //DrawPoint(shader, GameData::s_player1.GetMuzzleFlashSpawnMatrix(), RED);
    
    if (false)
    for (auto t : GameData::s_player1.m_character_model.m_animatedTransforms.worldspace)
    {
        Transform transform;
        transform.position = GameData::s_player1.GetPosition();
        transform.position.y -= 1;
        transform.rotation.y = GameData::s_player1.GetRotation().y + HELL_PI;
        DrawTangentDebugAxis(shader, transform.to_mat4() * t, 0.1f);
    }

	//DrawPoint(shader, GameData::s_player1.GetCasingSpawnLocation(), RED);

    // Bullet decals
    //if (s_showBulletDecalPositions)
		//for (auto& decal : GameData::s_bulletDecals)
			//DrawPoint(shader, decal.m_modelMatrix, RED);

    return;

    auto& glock = GameData::s_player1.m_HUD_Weapon;
    auto glockSkinnedModel = glock.GetSkinnedModelPtr();
    int BoneIndex = glockSkinnedModel->m_BoneMapping["barrel"];
    glm::mat4& BoneMatrix = GameData::s_player1.m_HUD_Weapon.m_animatedTransforms.local[BoneIndex];

    Transform weaponSwayTransform;
    Transform offsetTransform;
    offsetTransform.position = glm::vec3(0, -15, 10);

    glm::mat4 worldMatrix = glock.GetTransform().to_mat4() * weaponSwayTransform.to_mat4() * BoneMatrix * offsetTransform.to_mat4();

    float x = worldMatrix[3][0];
    float y = worldMatrix[3][1];
    float z = worldMatrix[3][2];
    glm::vec3 worldPosition = glm::vec3(x, y, z);

    Transform t(worldPosition);
    DrawPoint(shader, t.to_mat4(), RED);

    return;
    
    for (Door& door : GameData::s_doors)
    {
        PxVec3 p = door.m_rigid->getGlobalPose().p;
        PxQuat q = door.m_rigid->getGlobalPose().q;

        glm::vec3 position(p.x, p.y, p.z);
        glm::quat rotation(q.w, q.x, q.y, q.z);

        glm::mat4 m = Transform(position).to_mat4();

        DrawPoint(shader, m, RED);

        glm::mat4 m2 = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0, 0.1f));
        m2 *= glm::mat4_cast(rotation);

      //  DrawPoint(shader, m * m2, YELLOW);

        glm::mat4 mat = Util::PxMat44ToGlmMat4(door.m_rigid->getGlobalPose());

        glm::vec3 basisX = mat[0];
        glm::vec3 basisY = mat[1];
        glm::vec3 basisZ = mat[2];

        DrawPoint(shader, Transform(position + (basisZ * 0.1f)).to_mat4(), YELLOW);

       // glm::mat4 newPoint
        //Transform t = 
    }
   // PxVec3 pos = 

}

void Renderer::DrawFullScreenQuad(Shader* shader)
{
	static GLuint VAO_fullscreen = 0;

	if (VAO_fullscreen == 0) {
		float quadVertices[] = {
			// positions         texcoords
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
				1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
				1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
		};
		unsigned int VBO;
		glGenVertexArrays(1, &VAO_fullscreen);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO_fullscreen);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}

	shader->use();
	shader->setMat4("model", glm::mat4(1));

	glBindVertexArray(VAO_fullscreen);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

Player* Renderer::GetPlayerFromIndex(int index)
{
	if (index == 1)
		return &GameData::s_player1;
    if (index == 2)
        return &GameData::s_player2;
    else
        return nullptr; 
}

void Renderer::EnvMapPass()
{
    for (Light& light : GameData::s_lights)
    {
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] =
        {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };
        Transform viewPosTransform;

        Shader* shader = &s_env_map_shader;
        shader->use();
        //shader->setInt("equirectangularMap", 0);
        shader->setMat4("projection", captureProjection);
        shader->setVec3("lightPosition", light.m_position);
        shader->setFloat("lightAttenuationConstant", light.m_radius);
        shader->setFloat("lightAttenuationExp", light.m_magic);
        shader->setFloat("lightStrength", light.m_strength);
        shader->setVec3("lightColor", light.m_color);

        glViewport(0, 0, ENV_MAP_WIDTH, ENV_MAP_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, light.m_envMap.m_FboID);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        for (unsigned int i = 0; i < 6; ++i)
            shader->setMat4("captureViewMatrix[" + std::to_string(i) + "]", captureProjection * captureViews[i] * glm::inverse(Transform(light.m_position).to_mat4()));

        DrawScene(shader, RenderPass::ENV_MAP);

        //glBindTexture(GL_TEXTURE_CUBE_MAP, light.m_shadowMap.m_depthTexture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light.m_envMap.m_TexID);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, m_lightProbeStorage[lightIndex].CubeMap_TexID);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


    

        // SH texture
        glFinish();
        shader = &s_SH_shader;
        shader->use();
        glViewport(0, 0, 3, 3);
        glBindFramebuffer(GL_FRAMEBUFFER, light.m_envMap.SH_FboID);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light.m_envMap.m_TexID);
        glBindVertexArray(s_centeredQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    }
  //  RenderEnvMap(light, scene, i);
//
            //    RenderSphericalHarmonicsTexture(&m_spherical_harmonics_shader, i);
}

void Renderer::IndirectShadowMapPass()
{
    Shader* shader = &s_shadow_map_shader;
    shader->use();
    shader->setFloat("far_plane", SHADOW_FAR_PLANE);

    glDepthMask(true);
    glDisable(GL_BLEND);

    for (Light& light : GameData::s_lights)
	{
		if (!light.m_needsUpadte && !m_firstRenderLoop)
			continue;

        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        glBindFramebuffer(GL_FRAMEBUFFER, light.m_indirectShadowMap.m_ID);
        glEnable(GL_DEPTH_TEST);
         glClear(GL_DEPTH_BUFFER_BIT);

        for (unsigned int i = 0; i < 6; ++i)
            shader->setMat4("shadowMatrices[" + std::to_string(i) + "]", light.m_projectionTransforms[i]);
        shader->setVec3("lightPosition", light.m_position);
      
        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_NONE);
        glEnable(GL_CULL_FACE);
        DrawScene(shader, RenderPass::INDIRECT_SHADOW_MAP);
    } ;
}

void Renderer::ShadowMapPass()
{
    Shader* shader = &s_shadow_map_shader;
    shader->use();
    shader->setFloat("far_plane", SHADOW_FAR_PLANE);

    glDepthMask(true);
    glDisable(GL_BLEND);

    for (Light& light : GameData::s_lights)
    {
        if (!light.m_needsUpadte && !m_firstRenderLoop)
            continue;

        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        glBindFramebuffer(GL_FRAMEBUFFER, light.m_shadowMap.m_ID);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_DEPTH_BUFFER_BIT);

        for (unsigned int i = 0; i < 6; ++i)
            shader->setMat4("shadowMatrices[" + std::to_string(i) + "]", light.m_projectionTransforms[i]);
        shader->setVec3("lightPosition", light.m_position);

        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_NONE);
        glEnable(GL_CULL_FACE);
        DrawScene(shader, RenderPass::SHADOW_MAP);
    }
}


void Renderer::GeometryPass(int player, int renderWidth, int renderHeight)
{
    glBindFramebuffer(GL_FRAMEBUFFER, s_gBuffer.ID);

    unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT6 };
    glDrawBuffers(4, attachments);

    if(!GameData::s_splitScreen)
        glViewport(0, 0, CoreGL::s_currentWidth, CoreGL::s_currentHeight);
    else if (player == 1)
        glViewport(0, CoreGL::s_currentHeight / 2, CoreGL::s_currentWidth, CoreGL::s_currentHeight / 2);
    else if (player == 2)
        glViewport(0, 0, CoreGL::s_currentWidth, CoreGL::s_currentHeight / 2);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Shader* shader = &s_geometry_shader;
    shader->use();

    DrawScene(shader, RenderPass::GEOMETRY, player);


	// render player weapons
    shader->setFloat("u_gunMask", 1.0f);
    if (player == 1 && GameData::s_player1.m_isAlive || renderPlayerWeaponsAlways)
        GameData::s_player1.RenderWeaponModel(&s_geometry_shader);
    
	if (player == 2 && GameData::s_player2.m_isAlive || renderPlayerWeaponsAlways)
		GameData::s_player2.RenderWeaponModel(&s_geometry_shader);
	shader->setFloat("u_gunMask", 0.0f);
}

void Renderer::DecalPass(int playerIndex, int renderWidth, int renderHeight)
{
    // Bullet holes
    SetViewport(playerIndex);
    Player* player = GetPlayerFromIndex(playerIndex);
    Shader* shader = &s_decal_shader;
	shader->use();
    shader->setMat4("pv", player->GetProjectionViewMatrix());
	shader->setMat4("inverseProjectionMatrix", player->GetInverseProjectionMatrix());
	shader->setMat4("inverseViewMatrix", player->GetInverseViewMatrix());
	shader->setFloat("fullscreenWidth", CoreGL::s_currentWidth);
	shader->setFloat("fullscreenHeight", CoreGL::s_currentHeight);
	shader->setVec3("u_CameraFront", player->GetCameraFrontVector());
	shader->setFloat("u_alphaModifier", 0.5);
	shader->setVec3("u_ViewPos", player->GetViewPosition());
    if (!GameData::s_splitScreen)
        shader->setInt("u_playerIndex", 0);
    else
        shader->setInt("u_playerIndex", playerIndex);
	glBindFramebuffer(GL_FRAMEBUFFER, s_gBuffer.ID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, s_gBuffer.rboDepth);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, s_gBuffer.gNormal);
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("BulletHole_Plaster")->ID);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("BulletHole_Plaster_Mask")->ID);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);

	GameData::DrawInstancedBulletDecals(shader);

	//for (BulletDecal decal : GameData::s_bulletDecals)
	//	decal.Draw(shader);


    // Blood splatters
	shader = &s_blood_decal_shader;
	shader->use();
	shader->setMat4("pv", player->m_camera.m_projectionViewMatrix);
	shader->setMat4("inverseProjectionMatrix", glm::inverse(player->m_camera.m_projectionMatrix));
	shader->setMat4("inverseViewMatrix", glm::inverse(player->m_camera.m_viewMatrix));
	shader->setFloat("fullscreenWidth", CoreGL::s_currentWidth);
	shader->setFloat("fullscreenHeight", CoreGL::s_currentHeight);
	shader->setVec3("u_CameraFront", player->m_camera.m_Front);
	shader->setFloat("u_alphaModifier", 0.5);
	shader->setVec3("u_ViewPos", player->GetPosition() + glm::vec3(0, player->m_cameraViewHeight, 0));
	if (!GameData::s_splitScreen)
		shader->setInt("u_playerIndex", 0);
	else
		shader->setInt("u_playerIndex", playerIndex);
	glBindFramebuffer(GL_FRAMEBUFFER, s_gBuffer.ID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, s_gBuffer.rboDepth);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, s_gBuffer.gNormal);
	unsigned int attachments2[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments2);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	glBlendEquation(GL_FUNC_ADD);
	glEnablei(GL_BLEND, 1);
	glBlendFunci(1, GL_DST_COLOR, GL_SRC_COLOR);
	glDisablei(GL_BLEND, 1);
	glDisablei(GL_BLEND, 2);

	for (BloodDecal decal : GameData::s_bloodDecals)
		decal.Draw(shader);

    // Reset
	glDepthMask(GL_TRUE);
}

void Renderer::LightingPass(int player, int renderWidth, int renderHeight)
{
    ViewportSize viewportSize = ViewportSize::FULL_SCREEN;
    Shader* shader = &s_lighting_shader;
    shader->use();

    // Player 1 full screen
    if (player == 1 && !GameData::s_splitScreen) {
        shader->setInt("player", 0);
        shader->setVec3("camPos", GameData::s_player1.GetPosition() + glm::vec3(0, GameData::s_player1.m_cameraViewHeight, 0));
        viewportSize = ViewportSize::FULL_SCREEN;
    }
    // Player 1 split screen
    if (player == 1 && GameData::s_splitScreen) {
        shader->setInt("player", 1);
        shader->setVec3("camPos", GameData::s_player1.GetPosition() + glm::vec3(0, GameData::s_player1.m_cameraViewHeight, 0));
        viewportSize = ViewportSize::SPLITSCREEN_TOP;
    }
    // Player 2 split screen
    if (player == 2 && GameData::s_splitScreen) {
        shader->setInt("player", 2);
        shader->setVec3("camPos", GameData::s_player2.GetPosition() + glm::vec3(0, GameData::s_player2.m_cameraViewHeight, 0));
        viewportSize = ViewportSize::SPLITSCREEN_BOTTOM;
    }

    // Uniforms n shit
    shader->setFloat("shadow_map_far_plane", SHADOW_FAR_PLANE);
    shader->setFloat("screenWidth", CoreGL::s_currentWidth);
    shader->setFloat("screenHeight", CoreGL::s_currentHeight);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_gBuffer.gAlbedo);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, s_gBuffer.gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, s_gBuffer.gRMA);
    glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, s_gBuffer.rboDepth);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, s_brdfLUTTextureID);
    
    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(1, attachments);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE); // additive blending for multiple lights. you always forget how to do this.

    for (int i=0; i<GameData::s_lights.size(); i++)
    {
        Light& light = GameData::s_lights[i];
        shader->setInt("lightIndex", i);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light.m_shadowMap.m_depthTexture);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, light.m_envMap.SH_TexID);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light.m_indirectShadowMap.m_depthTexture);
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light.m_envMap.m_TexID);
        
        DrawViewportQuad(shader, viewportSize);
        //break;
    }

    // reset for the future. move this to whatever shader comes next
    unsigned int attachments2[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 , GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(5, attachments2);
    glEnable(GL_DEPTH_TEST); 
    glDisable(GL_BLEND);
}

void Renderer::MuzzleFlashPass(int playerIndex, int renderWidth, int renderHeight)
{
    // Get player pointer
    Player* player;
    if (playerIndex == 1)
        player = &GameData::s_player1;
    else if (playerIndex == 2)
        player = &GameData::s_player2;
    else
        return;

    // Bail if no flash
    if (player->m_muzzleFlashTimer < 0)
        return;
    if (player->m_muzzleFlashTimer > 1)
        return;

    s_muzzleFlash.m_CurrentTime = player->m_muzzleFlashTimer;

    glm::mat4 worldMatrix = player->GetMuzzleFlashSpawnMatrix();



    float x = worldMatrix[3][0];
    float y = worldMatrix[3][1];
    float z = worldMatrix[3][2];
    glm::vec3 worldPosition = glm::vec3(x, y, z);

    Camera* camera = &player->m_camera;

    Transform t;
    t.position = worldPosition;
    t.rotation = camera->m_transform.rotation;
   

    glBindFramebuffer(GL_FRAMEBUFFER, s_gBuffer.ID);

    SetViewport(playerIndex);



    // draw to lighting shader
    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT4 };
    glDrawBuffers(1, attachments);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    glm::mat4 proj = player->GetProjectionMatrix();
    glm::mat4 view = player->GetViewMatrix();
    glm::vec3 viewPos = player->GetViewPosition();

    Shader* shader = &s_animated_quad_shader;
    shader->use();
    shader->setMat4("u_MatrixProjection", proj);
    shader->setMat4("u_MatrixView", view);
    shader->setVec3("u_ViewPos", viewPos);

    for (int i = 0; i < GameData::s_lights.size(); i++)
    {
        Light& light = GameData::s_lights[i];
        shader->setVec3("lightPosition[" + std::to_string(i) + "]", light.m_position);
        shader->setFloat("lightRadius[" + std::to_string(i) + "]", light.m_radius);
        shader->setFloat("lightMagic[" + std::to_string(i) + "]", light.m_magic);
        shader->setFloat("lightStrength[" + std::to_string(i) + "]", light.m_strength);
        shader->setVec3("lightColor[" + std::to_string(i) + "]", light.m_color);
    }

   // Transform t;
    //t.position = glm::vec3(0, 1, 0);
    //t.position = worldPosition;
    //t.rotation = gameState->p_camera->m_transform.rotation;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("Muzzle1")->ID);

    s_muzzleFlash.Draw(shader, t, player->m_muzzleFlashRotation);
    glDisable(GL_BLEND); 
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);

    /*t.position = glm::vec3(0, 1, 0);
    t.rotation = glm::vec3(0, 0, 0);
    t.scale = glm::vec3(10);
    */

    /*glDisable(GL_DEPTH_TEST);

    unsigned int attachments2[7] = { GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4,
        GL_COLOR_ATTACHMENT5,
        GL_COLOR_ATTACHMENT6 };
    glDrawBuffers(7, attachments2);

    glEnable(GL_DEPTH_TEST);
   */
}

void Renderer::PostProcessingPass(int player)
{ 
    SetViewport(player);

    // DEPTH OF FIELD
    glBindFramebuffer(GL_FRAMEBUFFER, s_gBuffer.ID);
    bool DOF_showFocus = false;		//show debug focus point and focal range (red = focal point, green = focal range)
    bool DOF_vignetting = true;		//use optical lens vignetting?
    float DOF_vignout = 1.0;			//vignetting outer border
    float DOF_vignin = 0.0;			//vignetting inner border
    float DOF_vignfade = 122.0;		//f-stops till vignete fades
    float DOF_CoC = 0.03;				//circle of confusion size in mm (35mm film = 0.03mm)
    float DOF_maxblur = 0.35;			//clamp value of max blur (0.0 = no blur,1.0 default)
    int DOF_samples = 1;				//samples on the first ring
    int DOF_rings = 4;				//ring count
    float DOF_threshold = 0.5;		//highlight threshold;
    float DOF_gain = 2.0;				//highlight gain;
    float DOF_bias = 0.5;				//bokeh edge bias
    float DOF_fringe = 0.7;			//bokeh chromatic aberration/fringing
    Shader* shader = &s_postProcessingShader;
    shader->use();
    shader->setFloat("screenWidth", GetViewportWidth(player));
    shader->setFloat("screenHeight", GetViewportHeight(player));
    shader->setBool("showFocus", DOF_showFocus);
    shader->setBool("vignetting", DOF_vignetting);
    shader->setFloat("vignout", DOF_vignout);
    shader->setFloat("vignin", DOF_vignin);
    shader->setFloat("vignfade", DOF_vignfade);
    shader->setFloat("CoC", DOF_CoC);
    shader->setFloat("maxblur", DOF_maxblur);
    shader->setInt("samples", DOF_samples);
    shader->setInt("samples", DOF_samples);
    shader->setInt("rings", DOF_rings);
    shader->setFloat("threshold", DOF_threshold);
    shader->setFloat("gain", DOF_gain);
    shader->setFloat("bias", DOF_bias);
    shader->setFloat("fringe", DOF_fringe);
    shader->setFloat("znear", NEAR_PLANE);
    shader->setFloat("zfar", FAR_PLANE);

    int splitscreen_mode = 0;
    if (GameData::s_splitScreen && player == 1)
        splitscreen_mode = 1;
    else if (GameData::s_splitScreen && player == 2)
        splitscreen_mode = 2;
    shader->setInt("u_splitscreen_mode", splitscreen_mode);

    // Draw into gPostProcessing
    glDrawBuffer(GL_COLOR_ATTACHMENT5);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_gBuffer.gLighting);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, s_gBuffer.rboDepth);
    DrawViewportQuad(shader, GetViewportSize(player));

}

void Renderer::EmissiveBlurPass(int player, int renderWidth, int renderHeight, int levels)
{
	if (levels < 1)
		return;

    // down scale and blur the emissive texture thru ping pong shit 
	glDisable(GL_DEPTH_TEST);

    // Declare a pointer to a vector of blur buffres
    std::vector<BlurBuffer>* blurBuffers;

	if (player == 1)
		blurBuffers = &s_BlurBuffers_p1;
	else if (player == 2)
		blurBuffers = &s_BlurBuffers_p2;

	Shader* horizontalShader = &s_horizontal_blur_shader;
    Shader* verticalShader = &s_vertical_blur_shader;

	// Source
	glBindFramebuffer(GL_READ_FRAMEBUFFER, s_gBuffer.ID);
	glReadBuffer(GL_COLOR_ATTACHMENT6);
	int factor = 2;        

	// Destination
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (*blurBuffers)[0].ID);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

    int origin_X = 0;
    int origin_Y = 0;
	int viewportWidth = CoreGL::s_currentWidth;
	int viewportHeight = CoreGL::s_currentHeight;
    s_horizontal_blur_shader.use();
	s_horizontal_blur_shader.setInt("u_playerIndex", 0);
	s_vertical_blur_shader.use();
    s_vertical_blur_shader.setInt("u_playerIndex", 0);

    if (GameData::s_splitScreen)
	{
		if (player == 1) {
			origin_X = 0;
			origin_Y = CoreGL::s_currentHeight / 2;
			viewportWidth = CoreGL::s_currentWidth;
			viewportHeight = CoreGL::s_currentHeight / 2;
			s_horizontal_blur_shader.use();
			s_horizontal_blur_shader.setInt("u_playerIndex", 1);
			s_vertical_blur_shader.use();
			s_vertical_blur_shader.setInt("u_playerIndex", 1);
		}
		else if (player == 2) {
			origin_X = 0;
			origin_Y = 0;
			viewportWidth = CoreGL::s_currentWidth;
			viewportHeight = CoreGL::s_currentHeight / 2;
			s_horizontal_blur_shader.use();
			s_horizontal_blur_shader.setInt("u_playerIndex", 2);
            s_vertical_blur_shader.use();
			s_vertical_blur_shader.setInt("u_playerIndex", 2);
		}
    }

	// Blit from the original gEmissive texture into the first blur buffer
	int srcX_1 = origin_X;
	int srcY_1 = origin_Y;
	int srcX_2 = viewportWidth;
	int srcY_2 = origin_Y + viewportHeight;	
    int destX_1 = 0;
	int destY_1 = 0;
	int destX_2 = (*blurBuffers)[0].width;
	int destY_2 = (*blurBuffers)[0].height;
	glBlitFramebuffer(srcX_1, srcY_1, srcX_2, srcY_2, destX_1, destY_1, destX_2, destY_2, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	// Blur horizontal
	glViewport(0, 0, (*blurBuffers)[0].width, (*blurBuffers)[0].height);
	glBindFramebuffer(GL_FRAMEBUFFER, (*blurBuffers)[0].ID);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (*blurBuffers)[0].textureA);
	horizontalShader->use();
	horizontalShader->setFloat("targetWidth", viewportWidth / factor);
    DrawFullScreenQuad(horizontalShader);
	// Blur vertical
	glViewport(0, 0, (*blurBuffers)[0].width, (*blurBuffers)[0].height);
	glBindFramebuffer(GL_FRAMEBUFFER, (*blurBuffers)[0].ID);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (*blurBuffers)[0].textureB);
	verticalShader->use();
	verticalShader->setFloat("targetHeight", viewportHeight / factor);
	DrawViewportQuad(verticalShader, GetViewportSize(player));

    
	// second downscale //
	for (int i = 1; i < levels; i++)
	{
		// Source
		glBindFramebuffer(GL_READ_FRAMEBUFFER, (*blurBuffers)[i - 1].ID);
		glReadBuffer(GL_COLOR_ATTACHMENT0);

		// Destination
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (*blurBuffers)[i].ID);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		// Blit
		srcX_1 = 0;
		srcY_1 = 0;
		srcX_2 = (*blurBuffers)[i-1].width;
		srcY_2 = (*blurBuffers)[i-1].height;
		destX_1 = 0;
		destY_1 = 0;
		destX_2 = (*blurBuffers)[i].width;
		destY_2 = (*blurBuffers)[i].height;
		glBlitFramebuffer(srcX_1, srcY_1, srcX_2, srcY_2, destX_1, destY_1, destX_2, destY_2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        		
		// Blur horizontal
		glViewport(0, 0, (*blurBuffers)[i].width, (*blurBuffers)[i].height);
		glBindFramebuffer(GL_FRAMEBUFFER, (*blurBuffers)[i].ID);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (*blurBuffers)[i].textureA);
		horizontalShader->use();
		horizontalShader->setFloat("targetWidth", viewportWidth / factor);
		DrawFullScreenQuad(horizontalShader);

		// Blur vertical
		glViewport(0, 0, (*blurBuffers)[i].width, (*blurBuffers)[i].height);
		glBindFramebuffer(GL_FRAMEBUFFER, (*blurBuffers)[i].ID);
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (*blurBuffers)[i].textureB);
		verticalShader->use();
		verticalShader->setFloat("targetHeight", viewportHeight / factor);
		DrawFullScreenQuad(verticalShader);

		factor *= 2;
	}
	// return the viewport to it's orginal fucking size. this has stumped you for so long twice now. adding this will stop it.
	//glViewport(0, 0, CoreGL::s_windowWidth, CoreGL::s_windowHeight);*/
}


void Renderer::VolumetricBloodPass(int playerIndex, int renderWidth, int renderHeight)
{
    //static float animTime = 0;
	//animTime += game->m_frameTime;

	//if (Input::s_keyPressed[HELL_KEY_U])
	//	s_animTime = 0;

	glBindFramebuffer(GL_FRAMEBUFFER, s_gBuffer.ID);
    SetViewport(playerIndex);

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);


	//AssetManager::BindMaterial_0(AssetManager::GetMaterialIDByName("White"));
    Player* player = GetPlayerFromIndex(playerIndex);
    Shader* shader = &s_volumetric_blood_shader;
	shader->use();
	shader->setMat4("u_MatrixProjection", player->GetProjectionMatrix());
	shader->setMat4("u_MatrixView", player->GetViewMatrix());
	shader->setVec3("u_WorldSpaceCameraPos", player->GetViewPosition());

    
	for (int i = 0; i < GameData::s_volumetricBloodSplatters.size(); i++)
	{
        GameData::s_volumetricBloodSplatters[i].Draw(shader);
	}
}

void Renderer::DrawScene(Shader* shader, RenderPass renderPass, int player)
{
    // Indirect shadow map
    if (renderPass == RenderPass::INDIRECT_SHADOW_MAP)
    {
        for (auto& room : GameData::s_rooms) {
            room.DrawFloor(shader);
            room.DrawCeiling(shader);
            room.DrawWalls(shader);
        }
        for (auto& door : GameData::s_doors)
            door.Draw(shader);
        return;
    }

    // Shadow map
	else if (renderPass == RenderPass::SHADOW_MAP)
	{
		for (auto& room : GameData::s_rooms) {
			room.DrawFloor(shader);
			room.DrawCeiling(shader);
			room.DrawWalls(shader);
		}
		for (auto& door : GameData::s_doors)
			door.Draw(shader);

        for (EntityStatic entityStatic : GameData::s_staticEntities)
	        entityStatic.DrawEntity(shader);

		// Animated characters below
		shader->setBool("hasAnimation", true);
		shader->setMat4("model", glm::mat4(1));

		//for (GameCharacter& gameCharacter : Scene::s_gameCharacters)
		//    gameCharacter.RenderSkinnedModel(shader);
			
	//	glCullFace(GL_FRONT);
        if (GameData::s_player1.m_isAlive)
			GameData::s_player1.RenderCharacterModel(shader);
		if (GameData::s_player2.m_isAlive)
		    GameData::s_player2.RenderCharacterModel(shader);
	//	glCullFace(GL_BACK);
		return;
	}

    // Draw scene
    for (EntityStatic entityStatic : GameData::s_staticEntities)
        entityStatic.DrawEntity(shader);

    // Draw Rooms
    for (auto& room : GameData::s_rooms)
    {
        room.DrawFloor(shader);
        room.DrawCeiling(shader);
        room.DrawWalls(shader);
    }

    // Draw doors
    for (auto& door : GameData::s_doors)
        door.Draw(shader);

    if (renderPass == RenderPass::GEOMETRY) 
    {
		// Light bulbs
		shader->setBool("u_hasEmissive", true);
        for (int i = 0; i < GameData::s_lights.size(); i++) {
			Light* light = &GameData::s_lights[i];
			shader->setVec3("u_emissiveColor", light->m_color);
            light->Draw(shader);
        }
		shader->setBool("u_hasEmissive", false);
    }



    // Shells and bullet casings
	//for (BulletCasing& casing : GameData::s_bulletCasings)
	//	casing.Draw(shader);

	GameData::DrawInstancedGeometry(&s_instanced_geometry_shader);

    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glBlendEquation(GL_FUNC_ADD);


    glDisable(GL_BLEND);


    // Animated characters below
    shader->use();
    shader->setBool("hasAnimation", true);
    shader->setMat4("model", glm::mat4(1));




	Player* playerPtr = GetPlayerFromIndex(player);

    // corpse ragdolls
    if (renderPass != RenderPass::ENV_MAP)
        for (GameCharacter& gameCharacter : Scene::s_gameCharacters)
        {
			if (!playerPtr || playerPtr->m_corpse != &gameCharacter)  // don't draw your own corpse or you will see it while you got red vision
				gameCharacter.RenderSkinnedModel(shader);
        }
           
	// Player models
    if (renderPlayerCharacterModels) {
        if (player == 2 && GameData::s_player1.m_isAlive)
            GameData::s_player1.RenderCharacterModel(shader);
        if (player == 1 && GameData::s_player2.m_isAlive)
            GameData::s_player2.RenderCharacterModel(shader);
    }  

}