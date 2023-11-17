#pragma once
#include "D3D12VertexBuffer.h"
#include "D3D12IndexBuffer.h"

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT4 tangent;
    XMFLOAT2 texCoord;
    XMFLOAT4 color;
};

class D3D12Mesh
{
private:
	XMVECTOR position;
    Vertex* pVertices;
    UINT16* pIndices;
    UINT verticesSize;
    UINT verticesNum;
    UINT indicesSize;
    UINT indicesNum;

public:
    D3D12Mesh();
    ~D3D12Mesh();

    void SetVertices(Vertex* triangleVertices, UINT size);
    void SetIndices(UINT16* triangleIndices, UINT size);
    void CopyVertices(void* destination);
    void CopyIndices(void* destination);
    void CreateView();

    void AddGeometryBuffer(std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>&);

    inline const UINT GetVerticesSize() const { return verticesSize; }
    inline const UINT GetVerticesNum() const { return verticesNum; }
    inline const UINT GetIndicesSize() const { return indicesSize; }
    inline const UINT GetIndicesNum() const { return indicesNum; }
    inline const void* GetVerticesData() const { return pVertices; }
    inline const void* GetIndicesData() const { return pIndices; }

    std::unique_ptr<D3D12VertexBuffer> VertexBuffer;
    std::unique_ptr<D3D12IndexBuffer> IndexBuffer;
};