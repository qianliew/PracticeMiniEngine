#include "stdafx.h"
#include "D3D12Mesh.h"

void D3D12Mesh::SetVertices(Vertex* triangleVertices, UINT size)
{
    m_verticesSize = size;
    m_vertices.release();
    m_vertices = std::make_unique<Vertex*>((Vertex*)malloc(size));
    memcpy(*(m_vertices.get()), triangleVertices, m_verticesSize);

    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = m_verticesSize;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (VertexBuffer != nullptr)
    {
        VertexBuffer.release();
    }
    VertexBuffer = std::make_unique<D3D12VertexBuffer>(desc);
}

void D3D12Mesh::SetIndices(UINT16* triangleIndices, UINT size)
{
    m_indicesSize = size;
    m_indicesNum = size / sizeof(UINT16);
    m_indices.release();
    m_indices = std::make_unique<UINT16*>((UINT16*)malloc(size));
    memcpy(*(m_indices.get()), triangleIndices, m_indicesSize);

    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = m_indicesSize;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (IndexBuffer != nullptr)
    {
        IndexBuffer.release();
    }
    IndexBuffer = std::make_unique<D3D12IndexBuffer>(desc);
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

void D3D12Mesh::CreateView()
{
    // Initialize the vertex buffer view.
    VertexBuffer->CreateView();
    VertexBuffer->VertexBufferView.StrideInBytes = sizeof(Vertex);
    VertexBuffer->VertexBufferView.SizeInBytes = m_verticesSize;

    // Initialize the index buffer view.
    IndexBuffer->CreateView();
    IndexBuffer->IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    IndexBuffer->IndexBufferView.SizeInBytes = m_indicesSize;
}
