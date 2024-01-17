#include "stdafx.h"
#include "D3D12Mesh.h"

D3D12Mesh::D3D12Mesh() :
    pVertex(nullptr),
    pIndex(nullptr),
    pVertexBuffer(nullptr),
    pIndexBuffer(nullptr),
    position(XMVectorZero()),
    vertexSize(0),
    vertexCount(0),
    indexSize(0),
    indexCount(0)
{

}

D3D12Mesh::~D3D12Mesh()
{
    ResetVertices();
    ResetIndices();
}

void D3D12Mesh::ResetVertices()
{
    delete pVertex;
    delete pVertexBuffer;

    pVertex = nullptr;
    pVertexBuffer = nullptr;
}

void D3D12Mesh::ResetIndices()
{
    delete pIndex;
    delete pIndexBuffer;

    pIndex = nullptr;
    pIndexBuffer = nullptr;
}

void D3D12Mesh::SetVertices(Vertex* pInVertex, UINT64 size)
{
    ResetVertices();
    pVertexBuffer = new D3D12VertexBuffer();

    vertexSize = size;
    vertexCount = size / sizeof(Vertex);
    pVertex = (Vertex*)malloc(size);
    if (pVertex != nullptr)
    {
        memcpy(pVertex, pInVertex, vertexSize);
    }
}

void D3D12Mesh::SetIndices(UINT16* pInIndex, UINT64 size)
{
    ResetIndices();
    pIndexBuffer = new D3D12IndexBuffer();

    indexSize = size;
    indexCount = size / sizeof(UINT16);
    pIndex = (UINT16*)malloc(size);
    if (pIndex != nullptr)
    {
        memcpy(pIndex, pInIndex, indexSize);
    }
}

void D3D12Mesh::CopyVertices(void* destination)
{
    memcpy(destination, pVertex, vertexSize);
}

void D3D12Mesh::CopyIndices(void* destination)
{
    memcpy(destination, pIndex, indexSize);
}

const D3D12_RESOURCE_DESC D3D12Mesh::GetVertexResourceDesc()
{
    D3D12_RESOURCE_DESC resourceDesc;
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = vertexSize;
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
    resourceDesc.Width = indexSize;
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
    D3D12_VERTEX_BUFFER_VIEW vertexView = {};
    vertexView.StrideInBytes = sizeof(Vertex);
    vertexView.SizeInBytes = vertexSize;
    vertexView.BufferLocation = pVertexBuffer->GetResource()->GetGPUVirtualAddress();
    pVertexBuffer->SetVertexBufferView(vertexView);

    // Initialize the index buffer view.
    D3D12_INDEX_BUFFER_VIEW indexView = {};
    indexView.Format = DXGI_FORMAT_R16_UINT;
    indexView.SizeInBytes = indexSize;
    indexView.BufferLocation = pIndexBuffer->GetResource()->GetGPUVirtualAddress();
    pIndexBuffer->SetIndexBufferView(indexView);
}
