#pragma once
#include "Header.h"
#include "Renderer/Model.h"
#include "Renderer/Material.h"
#include "Renderer/Texture.h"
#include "Animation/SkinnedModel.h"
#include <filesystem>

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
        
        static  void LoadAssets();
        static Material* GetMaterialPtr(std::string name);
        static Texture* GetTexturePtr(std::string name);
        static Model* GetModelPtr(std::string name);
        static SkinnedModel* GetSkinnedModelPtr(std::string name);
        static Ragdoll* GetRagdollPtr(std::string name);
        // static SkinnedModel* GetSkinnedModel(std::string name);

        static void LoadSkinnedModel(std::string name, const char* filename);

    private: // fields
        static std::unordered_map<std::string, Model> m_models;
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
