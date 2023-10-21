#pragma once
#include "FBXImporter.h"
#include "D3D12Mesh.h"
#include "D3D12Texture.h"

using namespace std;

class D3D12Model : public Transform
{
private:
    shared_ptr<D3D12Mesh> mesh;
    shared_ptr<D3D12Texture> texture;
    shared_ptr<D3D12UploadBuffer> constant;

    char* m_meshPath;
    char* m_texturePath;

public:
    D3D12Model(char*, char*);

    void LoadModel(unique_ptr<FBXImporter>&);
    const shared_ptr<D3D12Mesh> GetMesh() const { return mesh; }
    const shared_ptr<D3D12Texture> GetTexture() const { return texture; }
    const shared_ptr<D3D12UploadBuffer> GetConstant() const { return constant; }
};
