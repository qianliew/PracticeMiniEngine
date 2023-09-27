#pragma once

#define MAX_UPLOAD_BUFFER_COUNT 4

enum class UploadBufferType
{
	Constant = 0,
	Index = 1,
	Vertex = 2,
	Texture = 3,
	Count = 4,
};

class D3D12BufferManager
{
private:
	ComPtr<ID3D12Device> m_device;
	D3D12UploadBuffer* UploadBufferPool[(int)UploadBufferType::Count][MAX_UPLOAD_BUFFER_COUNT];
	std::unordered_map<void*, D3D12DefaultBuffer*> DefaultBufferPool;

public:
	D3D12BufferManager(ComPtr<ID3D12Device>& device);
	~D3D12BufferManager();

	void AllocateUploadBuffer(D3D12UploadBuffer* &pBuffer, UploadBufferType type);
	void AllocateDefaultBuffer(D3D12Resource* pResource);
};
