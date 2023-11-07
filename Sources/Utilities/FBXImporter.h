#pragma once
#include <fbxsdk.h>
#include "D3D12Mesh.h"

using namespace std;
using namespace fbxsdk;

class FBXImporter
{
public:
    FBXImporter();
    ~FBXImporter();
    void InitializeSdkObjects();
    bool ImportFBX(std::wstring path);
    void LoadFBX(D3D12Mesh* mesh);
    void LoadContent(FbxScene* pScene, D3D12Mesh* mesh);
    void LoadContent(FbxNode* pNode, D3D12Mesh* mesh);

    // Mesh
    void LoadMesh(FbxNode* pNode, D3D12Mesh* mesh);

private:
    // FBX SDK objects
    FbxManager* m_fbxManager = nullptr;
    FbxScene* m_fbxScene = nullptr;
};