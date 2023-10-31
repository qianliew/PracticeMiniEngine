#pragma once
#include "FBXImporter.h"
#include "D3D12Mesh.h"
#include "D3D12Texture.h"

class D3D12Model : public Transform
{
private:
    D3D12Mesh* pMesh;
    LPCWSTR pMeshPath;

public:
    D3D12Model(UINT id, LPCWSTR);
    ~D3D12Model();

    void LoadModel(unique_ptr<FBXImporter>&);
    inline D3D12Mesh* GetMesh() const { return pMesh; }
};
