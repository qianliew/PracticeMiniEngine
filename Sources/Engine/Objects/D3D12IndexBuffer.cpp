#include "stdafx.h"
#include "D3D12IndexBuffer.h"

D3D12IndexBuffer::~D3D12IndexBuffer()
{
    delete View;
}

void D3D12IndexBuffer::CreateViewDesc()
{
    if (View == nullptr)
    {
        View = new D3D12IBV();
    }
    View->IndexBufferView.BufferLocation = ResourceLocation->Resource->GetGPUVirtualAddress();
}
