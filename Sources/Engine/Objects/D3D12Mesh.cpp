#include "stdafx.h"
#include "D3D12Mesh.h"

void D3D12Mesh::SetVertices(Vertex* triangleVertices, UINT size)
{
    m_verticesSize = size;
    m_vertices.release();
    m_vertices = std::make_unique<Vertex*>((Vertex*)malloc(size));
    memcpy(*(m_vertices.get()), triangleVertices, m_verticesSize);

    VertexBuffer = std::make_unique<D3D12VertexBuffer>();

    D3D12_RESOURCE_DESC* desc = new D3D12_RESOURCE_DESC();
    desc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc->Alignment = 0;
    desc->Width = m_verticesSize;
    desc->Height = 1;
    desc->DepthOrArraySize = 1;
    desc->MipLevels = 1;
    desc->Format = DXGI_FORMAT_UNKNOWN;
    desc->SampleDesc.Count = 1;
    desc->SampleDesc.Quality = 0;
    desc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc->Flags = D3D12_RESOURCE_FLAG_NONE;
    VertexBuffer->SetResourceDesc(desc);
}

void D3D12Mesh::SetIndices(UINT16* triangleIndices, UINT size)
{
    m_indicesSize = size;
    m_indicesNum = size / sizeof(UINT16);
    m_indices.release();
    m_indices = std::make_unique<UINT16*>((UINT16*)malloc(size));
    memcpy(*(m_indices.get()), triangleIndices, m_indicesSize);

    IndexBuffer = std::make_unique<D3D12IndexBuffer>();

    D3D12_RESOURCE_DESC* desc = new D3D12_RESOURCE_DESC();
    desc->Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc->Alignment = 0;
    desc->Width = m_indicesSize;
    desc->Height = 1;
    desc->DepthOrArraySize = 1;
    desc->MipLevels = 1;
    desc->Format = DXGI_FORMAT_UNKNOWN;
    desc->SampleDesc.Count = 1;
    desc->SampleDesc.Quality = 0;
    desc->Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc->Flags = D3D12_RESOURCE_FLAG_NONE;
    IndexBuffer->SetResourceDesc(desc);
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

void D3D12Mesh::CreateViewDesc()
{
    // Initialize the vertex buffer view.
    VertexBuffer->CreateViewDesc();
    VertexBuffer->View->VertexBufferView.StrideInBytes = sizeof(Vertex);
    VertexBuffer->View->VertexBufferView.SizeInBytes = m_verticesSize;

    // Initialize the index buffer view.
    IndexBuffer->CreateViewDesc();
    IndexBuffer->View->IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    IndexBuffer->View->IndexBufferView.SizeInBytes = m_indicesSize;
}
