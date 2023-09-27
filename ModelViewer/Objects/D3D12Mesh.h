#pragma once
#include "D3D12VertexBuffer.h"

using namespace DirectX;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 texCoord;
    XMFLOAT4 color;
};

class D3D12Mesh
{
private:
	XMVECTOR m_position;
    std::unique_ptr<Vertex*> m_vertices;
    std::unique_ptr<UINT16*> m_indices;
    UINT m_verticesSize;
    UINT m_indicesSize;
    UINT m_indicesNum;

public:
    void SetVertices(Vertex* triangleVertices, UINT size);
    void SetIndices(UINT16* triangleIndices, UINT size);
    UINT GetVerticesSize();
    UINT GetIndicesSize();
    UINT GetIndicesNum();
    void const* GetVerticesData();
    void const* GetIndicesData();
    void CopyVertices(void* destination);
    void CopyIndices(void* destination);
    void CreateView(ComPtr<ID3D12Device>& device, std::unique_ptr<D3D12DescriptorHeapManager>& manager);

    std::unique_ptr<D3D12VertexBuffer> VertexBuffer;
};