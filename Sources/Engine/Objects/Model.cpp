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
    XMFLOAT3 maxPos = pVertex->positionOS, minPos = pVertex->positionOS;
    for (UINT i = 0; i < pMesh->GetVerticesNum(); i++)
    {
        maxPos.x = max(maxPos.x, (pVertex + i)->positionOS.x);
        maxPos.y = max(maxPos.y, (pVertex + i)->positionOS.y);
        maxPos.z = max(maxPos.z, (pVertex + i)->positionOS.z);
        minPos.x = min(minPos.x, (pVertex + i)->positionOS.x);
        minPos.y = min(minPos.y, (pVertex + i)->positionOS.y);
        minPos.z = min(minPos.z, (pVertex + i)->positionOS.z);
    }

    pBoundingBox = new AABBBox(minPos, maxPos);
}
