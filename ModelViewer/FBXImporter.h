#pragma once
#include <fbxsdk.h>
#include "Mesh.h"

class FBXImporter
{
public:
    FBXImporter();
    ~FBXImporter();
    void InitializeSdkObjects();
    bool ImportFBX(char* path);
    void LoadFBX(const std::shared_ptr<Mesh>& mesh);
    void LoadContent(FbxScene* pScene, const std::shared_ptr<Mesh>& mesh);
    void LoadContent(FbxNode* pNode, const std::shared_ptr<Mesh>& mesh);

    // Mesh
    void LoadMesh(FbxNode* pNode, const std::shared_ptr<Mesh>& mesh);

private:
    // FBX SDK objects
    FbxManager* m_fbxManager = nullptr;
    FbxScene* m_fbxScene = nullptr;
};