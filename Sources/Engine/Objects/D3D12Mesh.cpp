#include "stdafx.h"
#include "D3D12Mesh.h"

D3D12Mesh::D3D12Mesh() :
    verticesSize(0),
    verticesNum(0),
    indicesSize(0),
    indicesNum(0)
{

}

D3D12Mesh::~D3D12Mesh()
{
    delete pVertices;
    delete pIndices;
    delete pVertexBuffer;
    delete pIndexBuffer;
    // delete pInstanceDescBuffer;

    pVertices = nullptr;
    pIndices = nullptr;
}

void D3D12Mesh::SetVertices(Vertex* triangleVertices, UINT size)
{
    UINT strideSize = sizeof(Vertex);
    verticesSize = size;
    verticesNum = size / strideSize;
    pVertices = (Vertex*)malloc(size);
    if (pVertices != nullptr)
    {
        memcpy(pVertices, triangleVertices, verticesSize);
    }

    if (pVertexBuffer != nullptr)
    {
        delete pVertexBuffer;
    }
    pVertexBuffer = new D3D12VertexBuffer();
}

void D3D12Mesh::SetIndices(UINT16* triangleIndices, UINT size)
{
    UINT strideSize = sizeof(UINT16);
    indicesSize = size;
    indicesNum = size / sizeof(UINT16);
    pIndices = (UINT16*)malloc(size);
    if (pIndices != nullptr)
    {
        memcpy(pIndices, triangleIndices, indicesSize);
    }

    if (pIndexBuffer != nullptr)
    {
        delete pIndexBuffer;
    }
    pIndexBuffer = new D3D12IndexBuffer();
}

void D3D12Mesh::CopyVertices(void* destination)
{
    memcpy(destination, pVertices, verticesSize);
}

void D3D12Mesh::CopyIndices(void* destination)
{
    memcpy(destination, pIndices, indicesSize);
}

const D3D12_RESOURCE_DESC D3D12Mesh::GetVertexResourceDesc()
{
    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = verticesSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    return resourceDesc;
}

const D3D12_RESOURCE_DESC D3D12Mesh::GetIndexResourceDesc()
{
    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = indicesSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    return resourceDesc;
}

void D3D12Mesh::CreateView()
{
    // Initialize the vertex buffer view.
    pVertexBuffer->CreateView();
    pVertexBuffer->VertexBufferView.StrideInBytes = sizeof(Vertex);
    pVertexBuffer->VertexBufferView.SizeInBytes = verticesSize;

    // Initialize the index buffer view.
    pIndexBuffer->CreateView();
    pIndexBuffer->IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    pIndexBuffer->IndexBufferView.SizeInBytes = indicesSize;
}
