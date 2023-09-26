#include "stdafx.h"
#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    delete m_vertexDesc;
    delete View;

    m_vertexDesc = nullptr;
    ResourceLocation = nullptr;
    View = nullptr;
}

void Mesh::SetVertices(Vertex* triangleVertices, UINT size)
{
    m_verticesSize = size;
    m_vertices.release();
    m_vertices = std::make_unique<Vertex*>((Vertex*)malloc(size));
    memcpy(*(m_vertices.get()), triangleVertices, m_verticesSize);

    m_vertexDesc = new D3D12_RESOURCE_DESC();
    m_vertexDesc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    m_vertexDesc->Alignment = 0;
    m_vertexDesc->Width = m_verticesSize;
    m_vertexDesc->Height = 1;
    m_vertexDesc->DepthOrArraySize = 1;
    m_vertexDesc->MipLevels = 1;
    m_vertexDesc->Format = DXGI_FORMAT_UNKNOWN;
    m_vertexDesc->SampleDesc.Count = 1;
    m_vertexDesc->SampleDesc.Quality = 0;
    m_vertexDesc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    m_vertexDesc->Flags = D3D12_RESOURCE_FLAG_NONE;

    View = new D3D12VBV();
}

void Mesh::SetIndices(UINT16* triangleIndices, UINT size)
{
    m_indicesSize = size;
    m_indicesNum = size / sizeof(UINT16);
    m_indices.release();
    m_indices = std::make_unique<UINT16*>((UINT16*)malloc(size));
    memcpy(*(m_indices.get()), triangleIndices, m_indicesSize);
}

UINT Mesh::GetVerticesSize()
{
    return m_verticesSize;
}

UINT Mesh::GetIndicesSize()
{
    return m_indicesSize;
}

UINT Mesh::GetIndicesNum()
{
    return m_indicesNum;
}

void const* Mesh::GetVerticesData()
{
    return *(m_vertices.get());
}

void const* Mesh::GetIndicesData()
{
    return *(m_indices.get());
}

D3D12_RESOURCE_DESC* Mesh::GetVertexDesc()
{
    return m_vertexDesc;
}

void Mesh::CopyVertices(void* destination)
{
    memcpy(destination, m_vertices.get(), m_verticesSize);
}

void Mesh::CopyIndices(void* destination)
{
    memcpy(destination, m_indices.get(), m_indicesSize);
}

void Mesh::CreateView(ComPtr<ID3D12Device>& device, std::unique_ptr<D3D12DescriptorHeapManager>& manager)
{
    // Initialize the vertex buffer view.
    View->VertexBufferView.BufferLocation = ResourceLocation->Resource->GetGPUVirtualAddress();
    View->VertexBufferView.StrideInBytes = sizeof(Vertex);
    View->VertexBufferView.SizeInBytes = m_verticesSize;
}
