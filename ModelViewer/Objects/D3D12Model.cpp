#include "stdafx.h"
#include "D3D12Model.h"

D3D12Model::D3D12Model(char* meshPath, char* texturePath)
{
    m_mesh = std::make_shared<D3D12Mesh>();
    m_texture = std::make_shared<D3D12TextureBuffer>();

    m_meshPath = meshPath;
    m_texturePath = texturePath;
}

void D3D12Model::LoadModel(unique_ptr<FBXImporter>& importer)
{
    if (importer->ImportFBX(m_meshPath))
    {
        importer->LoadFBX(m_mesh);
    }

    wchar_t wtext[100];
    size_t* ptr = new size_t();
    mbstowcs_s(ptr, wtext, m_texturePath, strlen(m_texturePath) + 1);
    delete ptr;

    m_texture->LoadTexture(wtext);
}

shared_ptr<D3D12Mesh> const D3D12Model::GetMesh()
{
    return m_mesh;
}

shared_ptr<D3D12TextureBuffer> const D3D12Model::GetTexture()
{
    return m_texture;
}
