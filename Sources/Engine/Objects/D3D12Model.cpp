#include "stdafx.h"
#include "D3D12Model.h"

D3D12Model::D3D12Model(UINT id, char* meshPath, char* texturePath) :
    Transform(id)
{
    mesh = std::make_shared<D3D12Mesh>();
    texture = std::make_shared<D3D12Texture>();

    pMeshPath = meshPath;
    pTexturePath = texturePath;
}

void D3D12Model::LoadModel(unique_ptr<FBXImporter>& importer)
{
    if (importer->ImportFBX(pMeshPath))
    {
        importer->LoadFBX(mesh);
    }

    wchar_t wtext[100];
    size_t* ptr = new size_t();
    mbstowcs_s(ptr, wtext, pTexturePath, strlen(pTexturePath) + 1);
    delete ptr;

    texture->LoadTexture(wtext);
    texture->CreateTexture(D3D12TextureType::ShaderResource);
}
