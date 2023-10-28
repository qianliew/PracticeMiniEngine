#include "stdafx.h"
#include "D3D12ConstantBuffer.h"

D3D12ConstantBuffer::D3D12ConstantBuffer(const D3D12_RESOURCE_DESC& desc, UINT inSize) :
	TD3D12Resource(desc),
	size(GET_CONSTANT_BUFFER_SIZE(inSize))
{

}

D3D12ConstantBuffer::~D3D12ConstantBuffer()
{

}

void D3D12ConstantBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
	view.SetResource(resourceLocation->Resource.Get());
	view.SetCPUHandle(handle);

	view.GetDesc().BufferLocation = resourceLocation->Resource->GetGPUVirtualAddress();
	view.GetDesc().SizeInBytes = size;

	view.CreateView(device);
}

void D3D12ConstantBuffer::CopyData(void const* source, size_t size)
{
	memcpy(startLocation, source, size);
}

void D3D12ConstantBuffer::SetStartLocation(void* location)
{
	startLocation = location;
}
