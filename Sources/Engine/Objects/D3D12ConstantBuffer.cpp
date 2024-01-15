#include "stdafx.h"
#include "D3D12ConstantBuffer.h"

D3D12ConstantBuffer::D3D12ConstantBuffer(UINT64 inSize) :
	size(GET_CONSTANT_BUFFER_SIZE(inSize))
{

}

D3D12ConstantBuffer::~D3D12ConstantBuffer()
{

}

void D3D12ConstantBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
	viewDesc.BufferLocation = pBuffer->GetResource()->GetGPUVirtualAddress();
	viewDesc.SizeInBytes = size;

	view = new D3D12CBV(viewDesc);
	view->SetResource(pBuffer->GetResource().Get());
	view->CreateView(device, handle);
}

void D3D12ConstantBuffer::CopyData(void const* source, size_t size)
{
	memcpy(pBuffer->GetLocation(), source, size);
}
