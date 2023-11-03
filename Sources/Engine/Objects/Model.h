#pragma once
#include "FBXImporter.h"
#include "D3D12Mesh.h"
#include "Material.h"

class Model : public Transform
{
private:
    D3D12Mesh* pMesh;
    Material* pMaterial;
    LPCWSTR pMeshPath;

public:
    Model(UINT id, LPCWSTR);
    ~Model();

    void LoadModel(unique_ptr<FBXImporter>&);
    void SetMaterial(Material*);

    inline D3D12Mesh* GetMesh() const { return pMesh; }
    inline Material* GetMaterial() const { return pMaterial; }
};
