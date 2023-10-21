#include "stdafx.h"
#include "D3D12UploadBuffer.h"

void D3D12UploadBuffer::CreateBuffer(ID3D12Device* device, UINT size)
{
	dataSize = size;
	bufferSize = size;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ResourceLocation->Resource.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(ResourceLocation->Resource->Map(0, &readRange, reinterpret_cast<void**>(&startLocation)));
	CreateViewDesc();
}

void D3D12UploadBuffer::CreateConstantBuffer(ID3D12Device* device, UINT size)
{
	dataSize = size;
	bufferSize = (dataSize + UPLOAD_BUFFER_ALIGNMENT - 1) & ~(UPLOAD_BUFFER_ALIGNMENT - 1);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ResourceLocation->Resource.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(ResourceLocation->Resource->Map(0, &readRange, reinterpret_cast<void**>(&startLocation)));
	CreateViewDesc();
}

void D3D12UploadBuffer::CopyData(void const* source)
{
	memcpy(startLocation, source, dataSize);
}

void D3D12UploadBuffer::CopyData(void const* source, size_t size)
{
	memcpy(startLocation, source, size);
}

void D3D12UploadBuffer::CreateViewDesc()
{
	view = new D3D12CBV();
	view->CBVDesc.BufferLocation = ResourceLocation->Resource->GetGPUVirtualAddress();
	view->CBVDesc.SizeInBytes = bufferSize;
}
