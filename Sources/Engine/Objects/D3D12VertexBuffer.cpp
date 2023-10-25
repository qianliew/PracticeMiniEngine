#include "stdafx.h"
#include "D3D12VertexBuffer.h"

D3D12VertexBuffer::D3D12VertexBuffer(UINT size) :
    D3D12Resource(size)
{

}

D3D12VertexBuffer::~D3D12VertexBuffer()
{

}

void D3D12VertexBuffer::CreateViewDesc()
{
    VertexBufferView.BufferLocation = resourceLocation->Resource->GetGPUVirtualAddress();
}
