#include "stdafx.h"
#include "D3D12Model.h"

D3D12Model::D3D12Model(UINT id, LPCWSTR meshPath) :
    Transform(id)
{
    pMesh = new D3D12Mesh();
    pMeshPath = meshPath;
}

D3D12Model::~D3D12Model()
{
    delete pMesh;
}

void D3D12Model::LoadModel(unique_ptr<FBXImporter>& importer)
{
    if ((pMeshPath != nullptr)
        && importer->ImportFBX(GetAssetPath(pMeshPath)))
    {
        importer->LoadFBX(pMesh);
    }
}
