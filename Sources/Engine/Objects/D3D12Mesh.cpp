#include "stdafx.h"
#include "D3D12Mesh.h"

D3D12Mesh::D3D12Mesh()
{

}

D3D12Mesh::~D3D12Mesh()
{
    delete pVertices;
    delete pIndices;

    pVertices = nullptr;
    pIndices = nullptr;
}

void D3D12Mesh::SetVertices(Vertex* triangleVertices, UINT size)
{
    verticesSize = size;
    pVertices = (Vertex*)malloc(size);
    if (pVertices != nullptr)
    {
        memcpy(pVertices, triangleVertices, verticesSize);
    }

    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = verticesSize;
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
    indicesSize = size;
    indicesNum = size / sizeof(UINT16);
    pIndices = (UINT16*)malloc(size);
    if (pIndices != nullptr)
    {
        memcpy(pIndices, triangleIndices, indicesSize);
    }

    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = indicesSize;
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
    return verticesSize;
}

UINT D3D12Mesh::GetIndicesSize()
{
    return indicesSize;
}

UINT D3D12Mesh::GetIndicesNum()
{
    return indicesNum;
}

void const* D3D12Mesh::GetVerticesData()
{
    return pVertices;
}

void const* D3D12Mesh::GetIndicesData()
{
    return pIndices;
}

void D3D12Mesh::CopyVertices(void* destination)
{
    memcpy(destination, pVertices, verticesSize);
}

void D3D12Mesh::CopyIndices(void* destination)
{
    memcpy(destination, pIndices, indicesSize);
}

void D3D12Mesh::CreateView()
{
    // Initialize the vertex buffer view.
    VertexBuffer->CreateView();
    VertexBuffer->VertexBufferView.StrideInBytes = sizeof(Vertex);
    VertexBuffer->VertexBufferView.SizeInBytes = verticesSize;

    // Initialize the index buffer view.
    IndexBuffer->CreateView();
    IndexBuffer->IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    IndexBuffer->IndexBufferView.SizeInBytes = indicesSize;
}
