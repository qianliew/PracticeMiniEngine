#include "stdafx.h"
#include "D3D12UploadBuffer.h"

D3D12UploadBuffer::D3D12UploadBuffer(const D3D12_RESOURCE_DESC& desc) :
	D3D12Buffer(desc)
{

}

D3D12UploadBuffer::~D3D12UploadBuffer()
{
	if (pResource != nullptr)
	{
		pResource->Unmap(0, nullptr);
	}
}

void D3D12UploadBuffer::CreateBuffer(
	ID3D12Device* device,
	const wchar_t* name)
{
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		ResourceState,
		nullptr,
		IID_PPV_ARGS(pResource.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(pResource->Map(0, &readRange, reinterpret_cast<void**>(&pLocation)));

	if (name)
	{
		pResource->SetName(name);
	}
}

void D3D12UploadBuffer::CopyData(void const* source, UINT64 size)
{
	memcpy(pLocation, source, size);
	bufferUsage += size;
}

void D3D12UploadBuffer::CopyData(void const* source, UINT64 size, UINT64 offset)
{
	memcpy((BYTE*)pLocation + offset, source, size);
	bufferUsage += size;
}

void D3D12UploadBuffer::ZeroData(UINT64 size)
{
	memset((BYTE*)pLocation, 0, size);
}
