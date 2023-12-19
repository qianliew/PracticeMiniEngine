#include "stdafx.h"
#include "Model.h"

Model::Model(UINT id, LPCWSTR meshPath) :
    Transform(id),
    pMeshPath(meshPath),
    pBoundingBox(nullptr)
{
    pMesh = new D3D12Mesh();
}

Model::~Model()
{
    delete pMesh;
    delete pBoundingBox;
}

void Model::LoadModel(unique_ptr<FBXImporter>& importer)
{
    if ((pMeshPath != nullptr)
        && importer->ImportFBX(GetAssetPath(pMeshPath)))
    {
        importer->LoadFBX(pMesh);
    }

    GenerateBoundingBox();
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

    GenerateBoundingBox();
}

void Model::SetMaterial(AbstractMaterial* material)
{
    pMaterial = material;
}

void Model::GenerateBoundingBox()
{
    if (pBoundingBox != nullptr)
    {
        delete pBoundingBox;
    }

    const Vertex* pVertex = static_cast<const Vertex*>(pMesh->GetVerticesData());

    D3D12_RAYTRACING_AABB aabb = {};
    aabb.MaxX = aabb.MinX = pVertex->positionOS.x;
    aabb.MaxY = aabb.MinY = pVertex->positionOS.y;
    aabb.MaxZ = aabb.MinZ = pVertex->positionOS.z;

    for (UINT i = 0; i < pMesh->GetVerticesNum(); i++)
    {
        aabb.MaxX = max(aabb.MaxX, (pVertex + i)->positionOS.x);
        aabb.MaxY = max(aabb.MaxY, (pVertex + i)->positionOS.y);
        aabb.MaxZ = max(aabb.MaxZ, (pVertex + i)->positionOS.z);
        aabb.MinX = min(aabb.MinX, (pVertex + i)->positionOS.x);
        aabb.MinY = min(aabb.MinY, (pVertex + i)->positionOS.y);
        aabb.MinZ = min(aabb.MinZ, (pVertex + i)->positionOS.z);
    }

    pBoundingBox = new AABBBox(aabb);
}
