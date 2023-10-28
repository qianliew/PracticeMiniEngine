#include "stdafx.h"
#include "D3D12VertexBuffer.h"

D3D12VertexBuffer::D3D12VertexBuffer(const D3D12_RESOURCE_DESC& desc) :
    D3D12Resource(desc)
{

}

D3D12VertexBuffer::~D3D12VertexBuffer()
{

}

void D3D12VertexBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{

}

void D3D12VertexBuffer::CreateView()
{
    VertexBufferView.BufferLocation = resourceLocation->Resource->GetGPUVirtualAddress();
}
