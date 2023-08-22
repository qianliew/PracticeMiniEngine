#include "stdafx.h"
#include "UploadBuffer.h"

UploadBuffer::~UploadBuffer()
{
	m_buffer->Unmap(0, nullptr);
	m_startLocation = nullptr;
}

ComPtr<ID3D12Resource> UploadBuffer::GetBuffer()
{
	return Buffer::GetBuffer();
}

void* UploadBuffer::GetStartLocation()
{
	return Buffer::m_startLocation;
}

UINT UploadBuffer::GetDataSize()
{
	return Buffer::GetDataSize();
}

void UploadBuffer::CreateBuffer(ID3D12Device* device, UINT size)
{
	m_dataSize = size;
	m_bufferSize = size;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_buffer.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_startLocation)));
}

void UploadBuffer::CreateConstantBuffer(ID3D12Device* device, UINT size)
{
	m_dataSize = size;
	m_bufferSize = (m_dataSize + UPLOAD_BUFFER_ALIGNMENT - 1) & ~(UPLOAD_BUFFER_ALIGNMENT - 1);

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_buffer.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(m_buffer->Map(0, &readRange, reinterpret_cast<void**>(&m_startLocation)));
}

void UploadBuffer::CopyData(void const* source)
{
	memcpy(m_startLocation, source, m_dataSize);
}
