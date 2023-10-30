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

    char* pMeshPath;
    char* pTexturePath;

public:
    D3D12Model(UINT id, char*, char*);
    ~D3D12Model();

    void LoadModel(unique_ptr<FBXImporter>&);
    const shared_ptr<D3D12Mesh> GetMesh() const { return mesh; }
    const shared_ptr<D3D12Texture> GetTexture() const { return texture; }
};
