#include "stdafx.h"
#include "D3D12IndexBuffer.h"

D3D12IndexBuffer::D3D12IndexBuffer(const D3D12_RESOURCE_DESC& desc) :
    D3D12Resource(desc)
{

}

D3D12IndexBuffer::~D3D12IndexBuffer()
{

}

void D3D12IndexBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{

}

void D3D12IndexBuffer::CreateView()
{
    IndexBufferView.BufferLocation = pResource->GetGPUVirtualAddress();
}
