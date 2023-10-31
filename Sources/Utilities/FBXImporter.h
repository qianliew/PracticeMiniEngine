#pragma once
#include <fbxsdk.h>
#include "D3D12Mesh.h"

using namespace std;

class FBXImporter
{
public:
    FBXImporter();
    ~FBXImporter();
    void InitializeSdkObjects();
    bool ImportFBX(std::wstring path);
    void LoadFBX(const std::shared_ptr<D3D12Mesh>& mesh);
    void LoadContent(FbxScene* pScene, const std::shared_ptr<D3D12Mesh>& mesh);
    void LoadContent(FbxNode* pNode, const std::shared_ptr<D3D12Mesh>& mesh);

    // Mesh
    void LoadMesh(FbxNode* pNode, const std::shared_ptr<D3D12Mesh>& mesh);

private:
    // FBX SDK objects
    FbxManager* m_fbxManager = nullptr;
    FbxScene* m_fbxScene = nullptr;
};