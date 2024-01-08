#include "stdafx.h"
#include "D3D12UploadBuffer.h"

D3D12UploadBuffer::D3D12UploadBuffer() :
	D3D12Buffer()
{

}

D3D12UploadBuffer::~D3D12UploadBuffer()
{
	if (ResourceLocation.Resource != nullptr)
	{
		ResourceLocation.Resource->Unmap(0, nullptr);
	}
}

void D3D12UploadBuffer::CreateBuffer(
	ID3D12Device* device,
	UINT64 size,
	D3D12_RESOURCE_STATES state,
	const wchar_t* name)
{
	bufferSize = size;
	D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		state,
		nullptr,
		IID_PPV_ARGS(ResourceLocation.Resource.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(ResourceLocation.Resource->Map(0, &readRange, reinterpret_cast<void**>(&startLocation)));

	if (name)
	{
		ResourceLocation.Resource->SetName(name);
	}
}

void D3D12UploadBuffer::CreateBuffer(
	ID3D12Device* device,
	const D3D12_RESOURCE_DESC& desc,
	D3D12_RESOURCE_STATES state,
	const wchar_t* name)
{
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		state,
		nullptr,
		IID_PPV_ARGS(ResourceLocation.Resource.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(ResourceLocation.Resource->Map(0, &readRange, reinterpret_cast<void**>(&startLocation)));

	if (name)
	{
		ResourceLocation.Resource->SetName(name);
	}
}

void D3D12UploadBuffer::CopyData(void const* source, UINT64 size)
{
	memcpy(startLocation, source, size);
	bufferUsage += size;
}

void D3D12UploadBuffer::CopyData(void const* source, UINT64 size, UINT64 offset)
{
	memcpy((BYTE*)startLocation + offset, source, size);
	bufferUsage += size;
}
