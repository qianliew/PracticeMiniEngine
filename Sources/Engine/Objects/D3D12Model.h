#pragma once
#include "FBXImporter.h"
#include "D3D12Mesh.h"
#include "D3D12Texture.h"

class D3D12Model : public Transform
{
private:
    shared_ptr<D3D12Mesh> mesh;
    shared_ptr<D3D12Texture> texture;

    LPCWSTR pMeshPath;
    LPCWSTR pTexturePath;

public:
    D3D12Model(UINT id, LPCWSTR, LPCWSTR);
    ~D3D12Model();

    void LoadModel(unique_ptr<FBXImporter>&);
    const shared_ptr<D3D12Mesh> GetMesh() const { return mesh; }
    const shared_ptr<D3D12Texture> GetTexture() const { return texture; }
};
