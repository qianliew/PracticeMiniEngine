#pragma once

#define MAX_UPLOAD_BUFFER_COUNT 10

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
	ComPtr<ID3D12Device> device;
	D3D12UploadBuffer* UploadBufferPool[(int)UploadBufferType::Count][MAX_UPLOAD_BUFFER_COUNT];
	std::unordered_map<const void*, D3D12DefaultBuffer*> DefaultBufferPool;
	std::shared_ptr<D3D12ConstantBuffer> globalConstantBuffer;
	std::unordered_map<UINT, D3D12ConstantBuffer*> perObjectConstantBuffers;

public:
	D3D12BufferManager(ComPtr<ID3D12Device>& device);
	~D3D12BufferManager();

	void AllocateUploadBuffer(D3D12UploadBuffer* &pBuffer, UploadBufferType type);
	void AllocateDefaultBuffer(D3D12Resource* pResource);

	void AllocateGlobalConstantBuffer(D3D12_CPU_DESCRIPTOR_HANDLE&);
	void AllocatePerObjectConstantBuffers(D3D12_CPU_DESCRIPTOR_HANDLE&, UINT);

	std::shared_ptr<D3D12ConstantBuffer> GetGlobalConstantBuffer() const { return globalConstantBuffer; }
	D3D12ConstantBuffer* GetPerObjectConstantBufferAtIndex(UINT);
};
