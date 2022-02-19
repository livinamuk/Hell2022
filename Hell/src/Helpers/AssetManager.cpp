#include "AssetManager.h"
#include "Renderer/Texture.h"
#include "Helpers/FileImporter.h"

std::unordered_map<std::string, Texture> AssetManager::m_textures;
std::unordered_map<std::string, Material> AssetManager::m_materials;
std::unordered_map<std::string, Model> AssetManager::m_models;
std::unordered_map<std::string, SkinnedModel*> AssetManager::m_skinnedModelPtrs;
std::vector<FileInfo> AssetManager::s_assetFilesToLoad;
std::string AssetManager::s_loadLog;

void AssetManager::DiscoverAssetFilenames()
{
    // load compressed texutures first
    std::string path = "res/textures/compressed/";

    /*    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        FileInfo info = GetFileInfo(entry);

        if (info.filetype == "dds")
            m_textures[info.filename] = Texture(info) ;
    }*/
        
    // then check regular file, and if name doesn't already exist, load it...
    path = "res/textures/";
    for (const auto& entry : std::filesystem::directory_iterator(path))            
        s_assetFilesToLoad.emplace_back(GetFileInfo(entry));

    path = "res/models/";
    for (const auto& entry : std::filesystem::directory_iterator(path))
        s_assetFilesToLoad.emplace_back(GetFileInfo(entry));
}

void AssetManager::LoadNextAssetToGL()
{
    if (s_assetFilesToLoad.size())
    {
        FileInfo info = s_assetFilesToLoad[0];                  // copy first element
        s_assetFilesToLoad.erase(s_assetFilesToLoad.begin());   // remove first element
        s_loadLog += "Loading " + info.fullpath + "\n";          // add to log

        if (info.filetype == "png" || info.filetype == "tga") {        // read texture from disk and load to GL
            if (m_textures.find(info.filename) == m_textures.end())
                m_textures[info.filename] = Texture(info);
        }

        if (info.filetype == "obj") 
        {
            bool createPhysicsMesh = false;

            if (info.filename == "Couch" ||
                info.filename == "TableSmall") 
                createPhysicsMesh = true;

            m_models[info.filename] = Model(info.fullpath.c_str(), createPhysicsMesh);
        }
    }
}

bool AssetManager::AssetsToLoad()
{
    return s_assetFilesToLoad.size();
}

void AssetManager::ForceLoadTexture(std::string filepath)
{
    Texture texture = Texture(filepath);
    m_textures[texture.m_fileNameInfo.filename] = texture;
}

void AssetManager::CreateMaterials()
{
    for (auto& it : m_textures)
    {
        Texture& texture = it.second;

        if (texture.m_fileNameInfo.materialType == "ALB")
        {
            std::string name = texture.m_fileNameInfo.filename.substr(0, texture.m_fileNameInfo.filename.length() - 4);

            Material material;
            material.ALB = texture.ID;
            material.RMA = GetTexturePtr(name + "_RMA")->ID;
            material.NRM = GetTexturePtr(name + "_NRM")->ID;
            m_materials[name] = material;

            std::cout << "Created Material: " << name << "\n";
        }
    }

}

void AssetManager::LoadRagdoll(std::string filepath)
{

}

void AssetManager::LoadAssets()
{
    LoadAllTextures();
    //CreateMaterials();
        
    /*if (!m_testEnviroment)
    {

        m_models["Couch"] = Model("Couch.obj", true);
        m_models["Cube"] = Model("Cube.obj");
        m_models["Sphere"] = Model("Sphere.obj");
        m_models["HalfSphere"] = Model("HalfSphere.obj");
        m_models["HalfSphere2"] = Model("HalfSphere2.obj");
        m_models["UncappedCylinder"] = Model("UncappedCylinder.obj");
        m_models["SphereLines"] = Model("SphereLines.obj");
        m_models["CubeLines"] = Model("CubeLines.obj");
        m_models["HalfSphereLines"] = Model("HalfSphereLines.obj");
        m_models["HalfSphereLines2"] = Model("HalfSphereLines2.obj");
        m_models["UncappedCylinderLines"] = Model("UncappedCylinderLines.obj");
        m_models["GlockBulletCasing"] = Model("BulletCasing.obj");
        m_models["DoorFrame"] = Model("DoorFrame.obj");
        m_models["Window"] = Model("Window.obj");
        m_models["WindowLines"] = Model("WindowLines.obj");
        m_models["WindowDouble"] = Model("WindowDouble.obj");
        m_models["WindowGlass"] = Model("WindowGlass.obj");
        m_models["WindowDoubleGlass"] = Model("WindowDoubleGlass.obj");
        m_models["WindowDoubleLines"] = Model("WindowDoubleLines.obj");
        m_models["TrimFloor"] = Model("TrimFloor.obj");
        m_models["TrimCeiling"] = Model("TrimCeiling.obj");
        m_models["DoorArc"] = Model("DoorArc.obj");
        //m_models["FracturedGlass"] = Model("FracturedGlass.obj");
        m_models["Door"] = Model("Door.obj");
        m_models["WeatherBoards"] = Model("WeatherBoards.obj");
        m_models["WeatherBoards_DoorGap"] = Model("WeatherBoards_DoorGap.obj");
        m_models["WeatherBoards_WindowGap"] = Model("WeatherBoards_WindowGap.obj");
        m_models["WeatherBoardStop"] = Model("WeatherBoardStop.obj");
        m_models["Light_01"] = Model("Light_01.obj");
        m_models["Light_02"] = Model("Light_02.obj");
        m_models["Light_03"] = Model("Light_03.obj");
        m_models["Light_04"] = Model("Light_04.obj");
        m_models["DoubleSidedQuad"] = Model("DoubleSidedQuad.obj");
        m_models["GlockOnly"] = Model("GlockOnly.obj");
        m_models["PulseRifle"] = Model("PulseRifle.obj", false);
        m_models["PulseRifle2"] = Model("PulseRifle2.obj", false);
        m_models["FaceHugger"] = Model("FaceHugger.obj", false);

        m_skinnedModels["NurseGuy"] = *FileImporter::LoadSkinnedModel("NurseGuy.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["NurseGuy"], "NursdeGuyShotgunIdle.fbx");

        m_skinnedModels["Glock"] = *FileImporter::LoadSkinnedModel("Glock.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_Fire0.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_Fire1.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_Fire2.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_Fire3.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_Idle.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_FirstEquip1.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_Equip.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_Equip2.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_Reload.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_EmptyReload.fbx");
        FileImporter::LoadAnimation(&m_skinnedModels["Glock"], "Glock_Walk.fbx");
    }

       

    //m_skinnedModels["Glock"].PrintMeshList();


    m_maleRagdoll.BuildFromJsonFile("ragdoll.json", Transform()); */
}


FileInfo AssetManager::GetFileInfo(const std::filesystem::directory_entry filepath)
{
    std::stringstream ss;
    ss << filepath.path();
    std::string fullpath = ss.str();
    // remove quotes at beginning and end
    fullpath = fullpath.substr(1);
    fullpath = fullpath.substr(0, fullpath.length() - 1);
    // isolate name
    std::string filename = fullpath.substr(fullpath.rfind("/") + 1);
    filename = filename.substr(0, filename.length() - 4);
    // isolate filetype
    std::string filetype = fullpath.substr(fullpath.length() - 3);
    // isolate direcetory
    std::string directory = fullpath.substr(0, fullpath.rfind("/") + 1);
    // material name
    std::string materialType = "NONE";
    if (filename.length() > 5) {
        std::string query = filename.substr(filename.length() - 3);
        if (query == "ALB" || query == "RMA" || query == "NRM")
            materialType = query;
    }
    // RETURN IT
    FileInfo info;
    info.fullpath = fullpath;
    info.filename = filename;
    info.filetype = filetype;
    info.directory = directory;
    info.materialType = materialType;
    return info;
}

void AssetManager::LoadAllTextures()
{
    // load compressed texutures first
    std::string path = "res/textures/compressed/";
       
    /* if (!m_testEnviroment) {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            FileInfo info = GetFileInfo(entry);

            if (info.filetype == "dds")
                m_textures[info.filename] = Texture(info);
        }
    }*/
       
    // std::cout << "Searching for non compressed textures...\n";
    // then check regular file, and if name doesn't already exist, load it...
    path = "res/textures/";
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        FileInfo info = GetFileInfo(entry);
        if (info.filetype == "png" || info.filetype == "tga") {
            if (m_textures.find(info.filename) == m_textures.end())
                m_textures[info.filename] = Texture(info);
        }
    }       
}

/*void AssetManager::CreateMaterials()
{
    for (auto& it : m_textures)
    {
        Texture& texture = it.second;

        if (texture.m_fileNameInfo.materialType == "ALB")
        {
            std::string name = texture.m_fileNameInfo.filename.substr(0, texture.m_fileNameInfo.filename.length() - 4);
               
            Material material;
            material.ALB = texture.ID;
            material.RMA = GetTexture(name + "_RMA")->ID;
            material.NRM = GetTexture(name + "_NRM")->ID;
            m_materials[name] = material;
        }
    }
}

Model* AssetManager::GetModel(std::string name)
{
    return &m_models[name];
}

Material* AssetManager::GetMaterial(std::string name)
{
    return &m_materials[name];
}
*/

Material* AssetManager::GetMaterialPtr(std::string name)
{
    return &m_materials[name];
}

Texture* AssetManager::GetTexturePtr(std::string name)
{
    return &m_textures[name];
}

Model* AssetManager::GetModelPtr(std::string name)
{
    return &m_models[name];
}

SkinnedModel* AssetManager::GetSkinnedModelPtr(std::string name)
{
    return m_skinnedModelPtrs[name];
}

void AssetManager::LoadSkinnedModel(std::string name, const char* filename)
{
    SkinnedModel* skinnedModel = FileImporter::LoadSkinnedModel(filename);
    m_skinnedModelPtrs[name] = skinnedModel;
}

/*   SkinnedModel* AssetManager::GetSkinnedModel(std::string name)
{
    return &m_skinnedModels[name];
}*/
