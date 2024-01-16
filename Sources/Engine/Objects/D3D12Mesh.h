#pragma once
#include "D3D12VertexBuffer.h"
#include "D3D12IndexBuffer.h"
#include "D3D12ShaderResourceBuffer.h"

using namespace DirectX;

class D3D12Mesh
{
private:
    Vertex* pVertex;
    UINT16* pIndex;
    D3D12VertexBuffer* pVertexBuffer;
    D3D12IndexBuffer* pIndexBuffer;

    XMVECTOR position;
    UINT64 vertexSize;
    UINT vertexCount;
    UINT64 indexSize;
    UINT indexCount;

public:
    D3D12Mesh();
    ~D3D12Mesh();

    void ResetVertices();
    void ResetIndices();
    void SetVertices(Vertex* pInVertex, UINT64 size);
    void SetIndices(UINT16* pInIndex, UINT64 size);
    void CopyVertices(void* destination);
    void CopyIndices(void* destination);
    const D3D12_RESOURCE_DESC GetVertexResourceDesc();
    const D3D12_RESOURCE_DESC GetIndexResourceDesc();
    void CreateView();

    inline const UINT64 GetVertexSize() const { return vertexSize; }
    inline const UINT GetVertexCount() const { return vertexCount; }
    inline const UINT64 GetIndexSize() const { return indexSize; }
    inline const UINT GetIndexCount() const { return indexCount; }
    inline const void* GetVertexData() const { return pVertex; }
    inline const void* GetIndexData() const { return pIndex; }

    inline D3D12VertexBuffer* GetVertexBuffer() const { return pVertexBuffer; }
    inline D3D12IndexBuffer* GetIndexBuffer() const { return pIndexBuffer; }
};