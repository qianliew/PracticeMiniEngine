#include "stdafx.h"
#include "D3D12UploadBuffer.h"

D3D12UploadBuffer::~D3D12UploadBuffer()
{
	ResourceLocation->Resource->Unmap(0, nullptr);
	m_startLocation = nullptr;
}

void* D3D12UploadBuffer::GetStartLocation()
{
	return D3D12Buffer::m_startLocation;
}

UINT D3D12UploadBuffer::GetDataSize()
{
	return D3D12Buffer::GetDataSize();
}

void D3D12UploadBuffer::CreateBuffer(ID3D12Device* device, UINT size)
{
	m_dataSize = size;
	m_bufferSize = size;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ResourceLocation->Resource.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(ResourceLocation->Resource->Map(0, &readRange, reinterpret_cast<void**>(&m_startLocation)));
}

void D3D12UploadBuffer::CreateConstantBuffer(ID3D12Device* device, UINT size)
{
	m_dataSize = size;
	m_bufferSize = (m_dataSize + UPLOAD_BUFFER_ALIGNMENT - 1) & ~(UPLOAD_BUFFER_ALIGNMENT - 1);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(ResourceLocation->Resource.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(ResourceLocation->Resource->Map(0, &readRange, reinterpret_cast<void**>(&m_startLocation)));
}

void D3D12UploadBuffer::CopyData(void const* source)
{
	memcpy(m_startLocation, source, m_dataSize);
}

void D3D12UploadBuffer::CopyData(void const* source, size_t size)
{
	memcpy(m_startLocation, source, size);
}
