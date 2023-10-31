#include "stdafx.h"
#include "D3D12Model.h"

D3D12Model::D3D12Model(UINT id, LPCWSTR meshPath, LPCWSTR texturePath) :
    Transform(id)
{
    mesh = std::make_shared<D3D12Mesh>();
    texture = std::make_shared<D3D12Texture>();

    pMeshPath = meshPath;
    pTexturePath = texturePath;
}

D3D12Model::~D3D12Model()
{

}

void D3D12Model::LoadModel(unique_ptr<FBXImporter>& importer)
{
    if ((pMeshPath != nullptr)
        && importer->ImportFBX(GetAssetPath(pMeshPath)))
    {
        importer->LoadFBX(mesh);
    }

    if (pTexturePath != nullptr)
    {
        texture->LoadTexture(GetAssetPath(pTexturePath).c_str());
        texture->CreateTexture(D3D12TextureType::ShaderResource);
    }
}
