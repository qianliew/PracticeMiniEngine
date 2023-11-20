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
    delete vertexBuffer;
    delete indexBuffer;
    // delete pInstanceDescBuffer;

    pVertices = nullptr;
    pIndices = nullptr;
}

void D3D12Mesh::SetVertices(Vertex* triangleVertices, UINT size)
{
    verticesSize = size;
    verticesNum = size / sizeof(Vertex);
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

    if (vertexBuffer != nullptr)
    {
        delete vertexBuffer;
    }
    vertexBuffer = new D3D12VertexBuffer(desc);
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

    if (indexBuffer != nullptr)
    {
        delete indexBuffer;
    }
    indexBuffer = new D3D12IndexBuffer(desc);
}

void D3D12Mesh::CopyVertices(void* destination)
{
    memcpy(destination, pVertices, verticesSize);
}

void D3D12Mesh::CopyIndices(void* destination)
{
    memcpy(destination, pIndices, indicesSize);
}

void D3D12Mesh::CreateView(BOOL isDXR)
{
    if (isDXR)
    {

    }
    else
    {
        // Initialize the vertex buffer view.
        vertexBuffer->CreateView();
        vertexBuffer->VertexBufferView.StrideInBytes = sizeof(Vertex);
        vertexBuffer->VertexBufferView.SizeInBytes = verticesSize;

        // Initialize the index buffer view.
        indexBuffer->CreateView();
        indexBuffer->IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
        indexBuffer->IndexBufferView.SizeInBytes = indicesSize;
    }
}

void D3D12Mesh::AddGeometryBuffer(std::vector<D3D12_RAYTRACING_GEOMETRY_DESC>& geometryDescs)
{
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    geometryDesc.Triangles.IndexBuffer = indexBuffer->GetResourceLocation().Resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.IndexCount = indicesNum;
    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
    geometryDesc.Triangles.Transform3x4 = 0;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.VertexCount = verticesNum;
    geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetResourceLocation().Resource->GetGPUVirtualAddress();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);

    geometryDescs.push_back(geometryDesc);
}
