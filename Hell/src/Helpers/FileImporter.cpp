#include "FileImporter.h"
#include "Util.h"

void FileImporter::VertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
{
    for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(IDs); i++) {
        if (Weights[i] == 0.0) {
            IDs[i] = BoneID;
            Weights[i] = Weight;
            return;
        }
    }
    return;
    // should never get here - more bones than we have space for
    assert(0);
}


SkinnedModel* FileImporter::LoadSkinnedModel(const char* filename)
{
    const aiScene* m_pScene;
    Assimp::Importer m_Importer;
   // m_Importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    SkinnedModel* skinnedModel = new SkinnedModel();

	skinnedModel->m_VAO = 0;
	ZERO_MEM(skinnedModel->m_Buffers);
	skinnedModel->m_NumBones = 0;
	skinnedModel->m_filename = filename;



    // Create the VAO
    glGenVertexArrays(1, &skinnedModel->m_VAO);
    glBindVertexArray(skinnedModel->m_VAO);

    // Create the buffers for the vertices attributes
    glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(skinnedModel->m_Buffers), skinnedModel->m_Buffers);

    bool Ret = false;

    std::string filepath = "res/models/";
    filepath += filename;

    const aiScene* tempScene = m_Importer.ReadFile(filepath.c_str(), aiProcess_LimitBoneWeights | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

    // aiProcess_OptimizeMeshes USE WITH BELOW 
    // aiProcess_OptimizeGraph 

    // aiProcess_PreTransformVertices

    //Getting corrupted later. So deep copying now.
    m_pScene = new aiScene(*tempScene);

    if (m_pScene) 
    {
        skinnedModel->m_GlobalInverseTransform = Util::aiMatrix4x4ToGlm(m_pScene->mRootNode->mTransformation);

        skinnedModel->m_GlobalInverseTransform = glm::inverse(skinnedModel->m_GlobalInverseTransform);



        Ret = InitFromScene(skinnedModel, m_pScene, filename);
    }
    else {
        printf("Error parsing '%s': '%s'\n", filename, m_Importer.GetErrorString());
    }

    /* std::cout << "\nLoaded: " << filename << "\n";
    std::cout << " " << m_pScene->mNumMeshes << " meshes\n";
    std::cout << " " << skinnedModel->m_NumBones << " bones\n";
        
    for (int i = 0; i < skinnedModel->m_meshEntries.size(); i++)  {
        std::cout << " -" << skinnedModel->m_meshEntries[i].Name << ": " << skinnedModel->m_meshEntries[i].NumIndices << " indices " << skinnedModel->m_meshEntries[i].BaseIndex << " base index " << skinnedModel->m_meshEntries[i].BaseVertex << " base vertex\n";
    }*/



        

    if (m_pScene->mNumCameras > 0)
        aiCamera* m_camera = m_pScene->mCameras[0];

    FindBindPoseTransforms(skinnedModel, m_pScene->mRootNode); // only used for debugging at this point

    GrabSkeleton(skinnedModel, m_pScene->mRootNode, -1);


     std::cout << "Loaded model " << skinnedModel->m_filename << " ("  << skinnedModel->m_BoneInfo.size() << " bones)\n";
     
     //std::cout << "m_GlobalInverseTransform\n";
	 //Util::PrintMat4(skinnedModel->m_GlobalInverseTransform);

	 skinnedModel->CalculateCameraBindposeTransform();

     for (auto b : skinnedModel->m_BoneInfo)
     {
         //std::cout << "-" << b.BoneName << "\n";
     }

    m_Importer.FreeScene();

    return skinnedModel;
}

bool FileImporter::InitFromScene(SkinnedModel* skinnedModel, const aiScene* pScene, const std::string& Filename)
{
    skinnedModel->m_meshEntries.resize(pScene->mNumMeshes);

    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec2> TexCoords;
    std::vector<VertexBoneData> Bones;
    std::vector<unsigned int> Indices;

    unsigned int NumVertices = 0;
    unsigned int NumIndices = 0;

    // Count the number of vertices and indices
    for (unsigned int i = 0; i < skinnedModel->m_meshEntries.size(); i++)
    {
        skinnedModel->m_meshEntries[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
        skinnedModel->m_meshEntries[i].BaseVertex = NumVertices;
        skinnedModel->m_meshEntries[i].BaseIndex = NumIndices;
        skinnedModel->m_meshEntries[i].Name = pScene->mMeshes[i]->mName.C_Str();

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices += skinnedModel->m_meshEntries[i].NumIndices;
    }

    // Reserve space in the vectors for the vertex attributes and indices
    Positions.reserve(NumVertices);
    Normals.reserve(NumVertices);
    TexCoords.reserve(NumVertices);
    Bones.resize(NumVertices);
    Indices.reserve(NumIndices);


    // Initialize the meshes in the scene one by one
    for (unsigned int i = 0; i < skinnedModel->m_meshEntries.size(); i++) {
        const aiMesh* paiMesh = pScene->mMeshes[i];
        InitMesh(skinnedModel, i, paiMesh, Positions, Normals, TexCoords, Bones, Indices);
    }


    glBindBuffer(GL_ARRAY_BUFFER, skinnedModel->m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, skinnedModel->m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, skinnedModel->m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TexCoords[0]) * TexCoords.size(), &TexCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, skinnedModel->m_Buffers[TANGENT_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(TANGENT_LOCATION);
    glVertexAttribPointer(TANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, skinnedModel->m_Buffers[BITANGENT_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Normals[0]) * Normals.size(), &Normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(BITANGENT_LOCATION);
    glVertexAttribPointer(BITANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, skinnedModel->m_Buffers[BONE_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Bones[0]) * Bones.size(), &Bones[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)16);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skinnedModel->m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

    //std::cout << "INDICES.size: " << Indices.size() << "\n";

    return true;
}


void FileImporter::InitMesh(SkinnedModel* skinnedModel, unsigned int MeshIndex,
    const aiMesh* paiMesh,
    std::vector<glm::vec3>& Positions,
    std::vector<glm::vec3>& Normals,
    std::vector<glm::vec2>& TexCoords,
    std::vector<VertexBoneData>& Bones,
    std::vector<unsigned int>& Indices)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
        



    // Populate the vertex attribute vectors
    for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
        const aiVector3D* pPos = &(paiMesh->mVertices[i]);
        const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
        const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0) ? &(paiMesh->mTextureCoords[0][i]) : &Zero3D;

        Positions.push_back(glm::vec3(pPos->x, pPos->y, pPos->z));
        Normals.push_back(glm::vec3(pNormal->x, pNormal->y, pNormal->z));
        TexCoords.push_back(glm::vec2(pTexCoord->x, pTexCoord->y));


        // this is my shit. my own copy of the data. 
        // umm deal with this later. as in removing all reliance on assimp data structures..
        // Also keep in mind this is only half complete and doesn't have bone shit.
        // you are just using it to add the mesh to bullet for blood lol.

        Vertex v;
        v.Position = Positions[i];
        v.Normal = Normals[i];
        v.TexCoords = TexCoords[i];
        //m_vertices.push_back(v);
    }

    LoadBones(skinnedModel, MeshIndex, paiMesh, Bones);

    // Populate the index buffer
    for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
        const aiFace& Face = paiMesh->mFaces[i];
        assert(Face.mNumIndices == 3);
        Indices.push_back(Face.mIndices[0]);
        Indices.push_back(Face.mIndices[1]);
        Indices.push_back(Face.mIndices[2]);
    }
}



void FileImporter::LoadBones(SkinnedModel* skinnedModel, unsigned int MeshIndex, const aiMesh* pMesh, std::vector<VertexBoneData>& Bones)
{
    for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
        unsigned int BoneIndex = 0;
        std::string BoneName(pMesh->mBones[i]->mName.data);

        if (skinnedModel->m_BoneMapping.find(BoneName) == skinnedModel->m_BoneMapping.end()) {
            // Allocate an index for a new bone
            BoneIndex = skinnedModel->m_NumBones;
            skinnedModel->m_NumBones++;
               
            SkinnedModel::BoneInfo bi;
            skinnedModel->m_BoneInfo.push_back(bi);
            skinnedModel->m_BoneInfo[BoneIndex].BoneOffset = Util::aiMatrix4x4ToGlm(pMesh->mBones[i]->mOffsetMatrix);
            skinnedModel->m_BoneInfo[BoneIndex].BoneName = BoneName;
            skinnedModel->m_BoneMapping[BoneName] = BoneIndex;
        }
        else {
            BoneIndex = skinnedModel->m_BoneMapping[BoneName];
        }

        for (unsigned int j = 0; j < pMesh->mBones[i]->mNumWeights; j++) {
            unsigned int VertexID = skinnedModel->m_meshEntries[MeshIndex].BaseVertex + pMesh->mBones[i]->mWeights[j].mVertexId;
            float Weight = pMesh->mBones[i]->mWeights[j].mWeight;
            Bones[VertexID].AddBoneData(BoneIndex, Weight);
        }
    }
}

void FileImporter::GrabSkeleton(SkinnedModel* skinnedModel, const aiNode* pNode, int parentIndex)
{
    // Ok. So this function walks the node tree and makes a direct copy and that becomes your custom skeleton.
    // This includes camera nodes and all that fbx pre rotation/translation bullshit. Hopefully assimp will fix that one day.

    Joint joint;
    joint.m_name = Util::CopyConstChar(pNode->mName.C_Str());
    joint.m_inverseBindTransform = Util::aiMatrix4x4ToGlm(pNode->mTransformation);
    joint.m_parentIndex = parentIndex;


	//std::cout << "--" << joint.m_name << "\n";
   // Util::PrintMat4(joint.m_inverseBindTransform);
	

    parentIndex = skinnedModel->m_joints.size(); // don't do your head in with why this works, just be thankful it does.

    skinnedModel->m_joints.push_back(joint);


    /*std::string NodeName(pNode->mName.data);
    if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
        unsigned int BoneIndex = m_BoneMapping[NodeName];
        m_BoneInfo[BoneIndex].DebugMatrix_BindPose = inverse(m_BoneInfo[BoneIndex].BoneOffset);
    }*/

    for (unsigned int i = 0; i < pNode->mNumChildren; i++)
        GrabSkeleton(skinnedModel, pNode->mChildren[i], parentIndex);
}

void FileImporter::FindBindPoseTransforms(SkinnedModel* skinnedModel, const aiNode* pNode)
{
    std::string NodeName(pNode->mName.data);

    if (skinnedModel->m_BoneMapping.find(NodeName) != skinnedModel->m_BoneMapping.end()) {
        unsigned int BoneIndex = skinnedModel->m_BoneMapping[NodeName];
        skinnedModel->m_BoneInfo[BoneIndex].DebugMatrix_BindPose = inverse(skinnedModel->m_BoneInfo[BoneIndex].BoneOffset);
    }

    for (unsigned int i = 0; i < pNode->mNumChildren; i++)
        FindBindPoseTransforms(skinnedModel, pNode->mChildren[i]);
}






void FileImporter::LoadAnimation(SkinnedModel* skinnedModel, const char* Filename)
{
    aiScene* m_pAnimationScene;
    Assimp::Importer m_AnimationImporter;

    Animation* animation = new Animation(Filename);
    // m_filename = Filename;

    std::string filepath = "res/animations/";
    filepath += Filename;

    // Try and load the animation
    const aiScene* tempAnimScene = m_AnimationImporter.ReadFile(filepath.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

    // Failed
    if (!tempAnimScene) {
        std::cout << "Could not load: " << Filename << "\n";
        assert(0);
    }

    // Success
    m_pAnimationScene = new aiScene(*tempAnimScene);
    if (m_pAnimationScene) {
        animation->m_duration = (float)m_pAnimationScene->mAnimations[0]->mDuration;
        animation->m_ticksPerSecond = m_pAnimationScene->mAnimations[0]->mTicksPerSecond;
         std::cout << "Loaded animation: " << Filename << "\n";
    }


    // Some other error possibilty
    else {
        printf("Error parsing '%s': '%s'\n", Filename, m_AnimationImporter.GetErrorString());
        assert(0);
    }

    // need to create an animation clip.
    // need to fill it with animation poses.
    aiAnimation* aiAnim = m_pAnimationScene->mAnimations[0];


    //std::cout << " numChannels:" << aiAnim->mNumChannels << "\n";

    // so iterate over each channel, and each channel is for each NODE aka joint.

    // Resize the vecotr big enough for each pose
    int nodeCount = aiAnim->mNumChannels;
    int poseCount = aiAnim->mChannels[0]->mNumPositionKeys;
      
    // trying the assimp way now. coz why fight it.
    for (int n = 0; n < nodeCount; n++)
    {
        const char* nodeName = Util::CopyConstChar(aiAnim->mChannels[n]->mNodeName.C_Str());

        AnimatedNode animatedNode(nodeName);
        animation->m_NodeMapping.emplace(nodeName, n);

        for (unsigned int p = 0; p < aiAnim->mChannels[n]->mNumPositionKeys; p++)
        {
            SQT sqt;
            aiVectorKey pos = aiAnim->mChannels[n]->mPositionKeys[p];
            aiQuatKey rot = aiAnim->mChannels[n]->mRotationKeys[p];
            aiVectorKey scale = aiAnim->mChannels[n]->mScalingKeys[p];

            sqt.positon = glm::vec3(pos.mValue.x, pos.mValue.y, pos.mValue.z);
            sqt.rotation = glm::quat(rot.mValue.w, rot.mValue.x, rot.mValue.y, rot.mValue.z);
            sqt.scale = scale.mValue.x;
            sqt.timeStamp = aiAnim->mChannels[n]->mPositionKeys[p].mTime;

            animation->m_finalTimeStamp = std::max(animation->m_finalTimeStamp, sqt.timeStamp);

            animatedNode.m_nodeKeys.push_back(sqt);
        }
        animation->m_animatedNodes.push_back(animatedNode);
    }

    // Store it
    skinnedModel->m_animations.emplace_back(animation);
    m_AnimationImporter.FreeScene();
}
