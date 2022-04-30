#pragma once
#include "Header.h"
#include "Renderer/Model.h"
#include "Renderer/Material.h"
#include "Renderer/Texture.h"
#include "Animation/SkinnedModel.h"
#include <filesystem>
#include "Renderer/ExrTexture.h"

    class AssetManager
    {
    public: // fields
        static std::string s_loadLog;

    public: // methods 
        //static AssetManager& Instance();
        static void ForceLoadTexture(std::string filepath);
        static void DiscoverAssetFilenames();
        static void LoadNextAssetToGL();
        static bool AssetsToLoad();
        static void CreateMaterials();
        static void LoadRagdoll(std::string filepath);
        static void LoadVolumetricBloodTextures();
        
        static  void LoadAssets();
        static Material* GetMaterialPtr(std::string name);
        static Texture* GetTexturePtr(std::string name);
        static Model* GetModelPtr(std::string name);
        static SkinnedModel* GetSkinnedModelPtr(std::string name);
        static Ragdoll* GetRagdollPtr(std::string name);

        static void LoadSkinnedModel(std::string name, const char* filename);

        static std::unordered_map<std::string, Model> m_models;

		static ExrTexture s_ExrTexture_pos;
		static ExrTexture s_ExrTexture_norm;
		static ExrTexture s_ExrTexture_pos4;
		static ExrTexture s_ExrTexture_norm4;
		static ExrTexture s_ExrTexture_pos6;
		static ExrTexture s_ExrTexture_norm6;
		static ExrTexture s_ExrTexture_pos7;
		static ExrTexture s_ExrTexture_norm7;
		static ExrTexture s_ExrTexture_pos8;
		static ExrTexture s_ExrTexture_norm8;
		static ExrTexture s_ExrTexture_pos9;
		static ExrTexture s_ExrTexture_norm9;

    private: // fields
        // static std::unordered_map<std::string, SkinnedModel> m_skinnedModels;
        static std::unordered_map<std::string, Material> m_materials;
        static std::unordered_map<std::string, Texture> m_textures;
        static std::unordered_map<std::string, SkinnedModel*> m_skinnedModelPtrs;
        static Ragdoll m_maleRagdoll;

    private: // methods
        static void LoadAllTextures();
        // static void CreateMaterials();
        static FileInfo GetFileInfo(const std::filesystem::directory_entry filepath);

        static std::vector<FileInfo> s_assetFilesToLoad;

        // bool m_testEnviroment = false;
    };
