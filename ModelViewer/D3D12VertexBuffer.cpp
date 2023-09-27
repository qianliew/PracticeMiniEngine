#include "stdafx.h"
#include "D3D12VertexBuffer.h"

D3D12VertexBuffer::~D3D12VertexBuffer()
{
    delete View;
}

void D3D12VertexBuffer::CreateView()
{
    if (View == nullptr)
    {
        View = new D3D12VBV();
    }
    View->VertexBufferView.BufferLocation = ResourceLocation->Resource->GetGPUVirtualAddress();
}
