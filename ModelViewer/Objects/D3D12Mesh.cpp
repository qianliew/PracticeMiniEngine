#include "stdafx.h"
#include "D3D12Mesh.h"

void D3D12Mesh::SetVertices(Vertex* triangleVertices, UINT size)
{
    m_verticesSize = size;
    m_vertices.release();
    m_vertices = std::make_unique<Vertex*>((Vertex*)malloc(size));
    memcpy(*(m_vertices.get()), triangleVertices, m_verticesSize);

    VertexBuffer = std::make_unique<D3D12VertexBuffer>();

    VertexBuffer->ResourceDesc = new D3D12_RESOURCE_DESC();
    VertexBuffer->ResourceDesc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    VertexBuffer->ResourceDesc->Alignment = 0;
    VertexBuffer->ResourceDesc->Width = m_verticesSize;
    VertexBuffer->ResourceDesc->Height = 1;
    VertexBuffer->ResourceDesc->DepthOrArraySize = 1;
    VertexBuffer->ResourceDesc->MipLevels = 1;
    VertexBuffer->ResourceDesc->Format = DXGI_FORMAT_UNKNOWN;
    VertexBuffer->ResourceDesc->SampleDesc.Count = 1;
    VertexBuffer->ResourceDesc->SampleDesc.Quality = 0;
    VertexBuffer->ResourceDesc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    VertexBuffer->ResourceDesc->Flags = D3D12_RESOURCE_FLAG_NONE;
}

void D3D12Mesh::SetIndices(UINT16* triangleIndices, UINT size)
{
    m_indicesSize = size;
    m_indicesNum = size / sizeof(UINT16);
    m_indices.release();
    m_indices = std::make_unique<UINT16*>((UINT16*)malloc(size));
    memcpy(*(m_indices.get()), triangleIndices, m_indicesSize);

    IndexBuffer = std::make_unique<D3D12IndexBuffer>();

    IndexBuffer->ResourceDesc = new D3D12_RESOURCE_DESC();
    IndexBuffer->ResourceDesc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    IndexBuffer->ResourceDesc->Alignment = 0;
    IndexBuffer->ResourceDesc->Width = m_indicesSize;
    IndexBuffer->ResourceDesc->Height = 1;
    IndexBuffer->ResourceDesc->DepthOrArraySize = 1;
    IndexBuffer->ResourceDesc->MipLevels = 1;
    IndexBuffer->ResourceDesc->Format = DXGI_FORMAT_UNKNOWN;
    IndexBuffer->ResourceDesc->SampleDesc.Count = 1;
    IndexBuffer->ResourceDesc->SampleDesc.Quality = 0;
    IndexBuffer->ResourceDesc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    IndexBuffer->ResourceDesc->Flags = D3D12_RESOURCE_FLAG_NONE;
}

UINT D3D12Mesh::GetVerticesSize()
{
    return m_verticesSize;
}

UINT D3D12Mesh::GetIndicesSize()
{
    return m_indicesSize;
}

UINT D3D12Mesh::GetIndicesNum()
{
    return m_indicesNum;
}

void const* D3D12Mesh::GetVerticesData()
{
    return *(m_vertices.get());
}

void const* D3D12Mesh::GetIndicesData()
{
    return *(m_indices.get());
}

void D3D12Mesh::CopyVertices(void* destination)
{
    memcpy(destination, m_vertices.get(), m_verticesSize);
}

void D3D12Mesh::CopyIndices(void* destination)
{
    memcpy(destination, m_indices.get(), m_indicesSize);
}

void D3D12Mesh::CreateView(ComPtr<ID3D12Device>& device, std::unique_ptr<D3D12DescriptorHeapManager>& manager)
{
    // Initialize the vertex buffer view.
    VertexBuffer->View->VertexBufferView.BufferLocation = VertexBuffer->ResourceLocation->Resource->GetGPUVirtualAddress();
    VertexBuffer->View->VertexBufferView.StrideInBytes = sizeof(Vertex);
    VertexBuffer->View->VertexBufferView.SizeInBytes = m_verticesSize;

    // Initialize the index buffer view.
    IndexBuffer->View->IndexBufferView.BufferLocation = IndexBuffer->ResourceLocation->Resource->GetGPUVirtualAddress();
    IndexBuffer->View->IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    IndexBuffer->View->IndexBufferView.SizeInBytes = m_indicesSize;
}
