#include "stdafx.h"
#include "D3D12IndexBuffer.h"

D3D12IndexBuffer::D3D12IndexBuffer(UINT size) :
    D3D12Resource(size)
{

}

D3D12IndexBuffer::~D3D12IndexBuffer()
{

}

void D3D12IndexBuffer::CreateViewDesc()
{
    IndexBufferView.BufferLocation = resourceLocation->Resource->GetGPUVirtualAddress();
}
