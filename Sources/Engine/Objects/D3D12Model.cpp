#include "stdafx.h"
#include "D3D12Model.h"

D3D12Model::D3D12Model(char* meshPath, char* texturePath)
{
    mesh = std::make_shared<D3D12Mesh>();
    texture = std::make_shared<D3D12Texture>();
    constant = std::make_shared<D3D12UploadBuffer>();

    m_meshPath = meshPath;
    m_texturePath = texturePath;
}

void D3D12Model::LoadModel(unique_ptr<FBXImporter>& importer)
{
    if (importer->ImportFBX(m_meshPath))
    {
        importer->LoadFBX(mesh);
    }

    wchar_t wtext[100];
    size_t* ptr = new size_t();
    mbstowcs_s(ptr, wtext, m_texturePath, strlen(m_texturePath) + 1);
    delete ptr;

    texture->LoadTexture(wtext);
}
