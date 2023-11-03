#include "stdafx.h"
#include "Model.h"

Model::Model(UINT id, LPCWSTR meshPath) :
    Transform(id),
    pMeshPath(meshPath)
{
    pMesh = new D3D12Mesh();
}

Model::~Model()
{
    delete pMesh;
}

void Model::LoadModel(unique_ptr<FBXImporter>& importer)
{
    if ((pMeshPath != nullptr)
        && importer->ImportFBX(GetAssetPath(pMeshPath)))
    {
        importer->LoadFBX(pMesh);
    }
}

void Model::SetMaterial(Material* material)
{
    pMaterial = material;
}
