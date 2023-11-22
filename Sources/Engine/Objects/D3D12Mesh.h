#pragma once
#include "D3D12VertexBuffer.h"
#include "D3D12IndexBuffer.h"
#include "D3D12ShaderResourceBuffer.h"

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

    D3D12VertexBuffer* pVertexBuffer;
    D3D12IndexBuffer* pIndexBuffer;

public:
    D3D12Mesh();
    ~D3D12Mesh();

    void SetVertices(Vertex* triangleVertices, UINT size);
    void SetIndices(UINT16* triangleIndices, UINT size);
    void CopyVertices(void* destination);
    void CopyIndices(void* destination);
    void CreateView();

    inline const UINT GetVerticesSize() const { return verticesSize; }
    inline const UINT GetVerticesNum() const { return verticesNum; }
    inline const UINT GetIndicesSize() const { return indicesSize; }
    inline const UINT GetIndicesNum() const { return indicesNum; }
    inline const void* GetVerticesData() const { return pVertices; }
    inline const void* GetIndicesData() const { return pIndices; }

    inline D3D12VertexBuffer* GetVertexBuffer() const { return pVertexBuffer; }
    inline D3D12IndexBuffer* GetIndexBuffer() const { return pIndexBuffer; }
};