#pragma once

#define MAX_TEMP_UPLOAD_BUFFER_COUNT 100
#define MAX_READBACK_BUFFER_COUNT 10
#define MAX_UPLOAD_BUFFER_COUNT 10

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
	D3D12UploadBuffer* tempUploadBufferPool[MAX_TEMP_UPLOAD_BUFFER_COUNT];
	D3D12UploadBuffer* uploadBufferPool[MAX_UPLOAD_BUFFER_COUNT];
	D3D12ReadbackBuffer* readbackBufferPool[MAX_READBACK_BUFFER_COUNT];
	std::unordered_map<const void*, D3D12DefaultBuffer*> defaultBufferPool;
	D3D12ConstantBuffer* globalConstantBuffer;
	std::map<UINT, D3D12ConstantBuffer*> perObjectConstantBuffers;

public:
	D3D12BufferManager(ComPtr<ID3D12Device>& device);
	~D3D12BufferManager();

	void AllocateTempUploadBuffer(
		D3D12UploadBuffer* pBuffer,
		UINT64 size,
		const wchar_t* name = nullptr);
	void ReleaseTempUploadBuffer();

	void AllocateUploadBuffer(
		D3D12UploadBuffer* pBuffer,
		UINT64 size,
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_GENERIC_READ,
		const wchar_t* name = nullptr);
	void AllocateReadbackBuffer(
		D3D12ReadbackBuffer* pBuffer,
		UINT64 size,
		const wchar_t* name = nullptr);
	void AllocateDefaultBuffer(
		D3D12Resource* pResource,
		const D3D12_RESOURCE_DESC& desc,
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST,
		const wchar_t* name = nullptr,
		const D3D12_CLEAR_VALUE* clearValue = nullptr);

	void AllocateGlobalConstantBuffer();
	void AllocatePerObjectConstantBuffers(UINT offset);

	inline D3D12ConstantBuffer* GetGlobalConstantBuffer() const { return globalConstantBuffer; }
	D3D12ConstantBuffer* GetPerObjectConstantBufferAtIndex(UINT);
};
