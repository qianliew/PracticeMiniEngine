#include "stdafx.h"
#include "D3D12IndexBuffer.h"

D3D12IndexBuffer::D3D12IndexBuffer(UINT size) :
    D3D12Resource(size)
{
    View = new D3D12IBV();
}

D3D12IndexBuffer::~D3D12IndexBuffer()
{
    delete View;
}

void D3D12IndexBuffer::CreateViewDesc()
{
    View->IndexBufferView.BufferLocation = resourceLocation->Resource->GetGPUVirtualAddress();
}
