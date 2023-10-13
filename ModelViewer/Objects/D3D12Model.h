#pragma once
#include "FBXImporter.h"
#include "D3D12Mesh.h"
#include "D3D12TextureBuffer.h"

using namespace std;

class D3D12Model
{
private:
    shared_ptr<D3D12Mesh> m_mesh;
    shared_ptr<D3D12TextureBuffer> m_texture;

    char* m_meshPath;
    char* m_texturePath;

public:
    D3D12Model(char*, char*);

    void LoadModel(unique_ptr<FBXImporter>&);
    shared_ptr<D3D12Mesh> const GetMesh();
    shared_ptr<D3D12TextureBuffer> const GetTexture();
};