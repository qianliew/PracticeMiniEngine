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

void Model::CreatePlane()
{
    const int indexNum = 6;
    UINT16 pIndex[] = { 0, 1, 2, 0, 2, 3 };
    Vertex pVertex[] =
    {
        { XMFLOAT3{ 1, 1, 0 }, XMFLOAT3{ 0, 0, 1 }, XMFLOAT4{ 1, 0, 0, 0 }, XMFLOAT2{ 1, 0 }, XMFLOAT4{ 0, 0, 0, 0 } },
        { XMFLOAT3{ -1, 1, 0 }, XMFLOAT3{ 0, 0, 1 }, XMFLOAT4{ 1, 0, 0, 0 }, XMFLOAT2{ 0, 0 }, XMFLOAT4{ 0, 0, 0, 0 } },
        { XMFLOAT3{ -1, -1, 0 }, XMFLOAT3{ 0, 0, 1 }, XMFLOAT4{ 1, 0, 0, 0 }, XMFLOAT2{ 0, 1 }, XMFLOAT4{ 0, 0, 0, 0 } },
        { XMFLOAT3{ 1, -1, 0 }, XMFLOAT3{ 0, 0, 1 }, XMFLOAT4{ 1, 0, 0, 0 }, XMFLOAT2{ 1, 1 }, XMFLOAT4{ 0, 0, 0, 0 } },
    };

    pMesh->SetIndices(pIndex, sizeof(pIndex));
    pMesh->SetVertices(pVertex, sizeof(pVertex));
}

void Model::SetMaterial(AbstractMaterial* material)
{
    pMaterial = material;
}
