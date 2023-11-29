#pragma once
#include "FBXImporter.h"
#include "D3D12Mesh.h"
#include "Transform.h"
#include "AbstractMaterial.h"

class Model : public Transform
{
private:
    D3D12Mesh* pMesh;
    AbstractMaterial* pMaterial;
    LPCWSTR pMeshPath;

public:
    Model(UINT id, LPCWSTR);
    ~Model();

    void LoadModel(unique_ptr<FBXImporter>&);
    void CreatePlane();
    void SetMaterial(AbstractMaterial*);

    inline D3D12Mesh* GetMesh() const { return pMesh; }
    inline AbstractMaterial* GetMaterial() const { return pMaterial; }
};
