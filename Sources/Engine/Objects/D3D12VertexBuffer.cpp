#include "stdafx.h"
#include "D3D12VertexBuffer.h"

D3D12VertexBuffer::D3D12VertexBuffer(UINT size) :
    D3D12Resource(size)
{
    View = new D3D12VBV();
}

D3D12VertexBuffer::~D3D12VertexBuffer()
{
    delete View;
}

void D3D12VertexBuffer::CreateViewDesc()
{
    View->VertexBufferView.BufferLocation = resourceLocation->Resource->GetGPUVirtualAddress();
}
