#pragma once

#define MAX_UPLOAD_BUFFER_COUNT 100

enum class UploadBufferType
{
	Constant = 0,
	Index = 1,
	Vertex = 2,
	Texture = 3,
	Count = 4,
};

class D3D12DescriptorHeapManager;

class D3D12BufferManager
{
private:
	ComPtr<ID3D12Device> pDevice;
	D3D12UploadBuffer* UploadBufferPool[MAX_UPLOAD_BUFFER_COUNT];
	std::unordered_map<const void*, D3D12DefaultBuffer*> DefaultBufferPool;
	D3D12ConstantBuffer* globalConstantBuffer;
	std::map<UINT, D3D12ConstantBuffer*> perObjectConstantBuffers;

public:
	D3D12BufferManager(ComPtr<ID3D12Device>& device);
	~D3D12BufferManager();

	void AllocateUploadBuffer(D3D12UploadBuffer* pBuffer, UINT64 size);
	void ReleaseUploadBuffer();
	void AllocateDefaultBuffer(
		D3D12Resource* pResource,
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST,
		const D3D12_CLEAR_VALUE* clearValue = nullptr);

	inline void AllocateUAVBuffer(UINT64 bufferSize, ID3D12Resource** ppResource, D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_COMMON, const wchar_t* resourceName = nullptr)
	{
		auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		ThrowIfFailed(pDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			initialResourceState,
			nullptr,
			IID_PPV_ARGS(ppResource)));
		if (resourceName)
		{
			(*ppResource)->SetName(resourceName);
		}
	}

	void AllocateGlobalConstantBuffer();
	void AllocatePerObjectConstantBuffers(UINT offset);

	D3D12ConstantBuffer* GetGlobalConstantBuffer() const { return globalConstantBuffer; }
	D3D12ConstantBuffer* GetPerObjectConstantBufferAtIndex(UINT);
};
