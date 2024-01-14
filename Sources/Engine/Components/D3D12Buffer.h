#pragma once

class D3D12Buffer
{
protected:
	ComPtr<ID3D12Resource> pResource;
	void* pLocation;

	const D3D12_RESOURCE_DESC resourceDesc;
	D3D12_RESOURCE_STATES resourceState;
	UINT64 bufferUsage;

public:
	D3D12Buffer() = delete;
	D3D12Buffer(const D3D12Buffer&) = delete;
	D3D12Buffer& operator= (const D3D12Buffer&) = delete;
	D3D12Buffer(const D3D12_RESOURCE_DESC& desc);
	virtual ~D3D12Buffer();

	virtual void CreateBuffer(
		ID3D12Device* device,
		const wchar_t* name,
		const D3D12_CLEAR_VALUE* clearValue,
		const D3D12_RESOURCE_STATES state) = 0;

	inline ComPtr<ID3D12Resource>& GetResource() { return pResource; }
	inline void* GetLocation() { return pLocation; }
	inline const D3D12_RESOURCE_DESC& GetResourceDesc() const { return resourceDesc; }
	inline const D3D12_RESOURCE_STATES GetResourceState() const { return resourceState; }
	inline const UINT64 GetBufferSize() const { return resourceDesc.Width; }
	inline const UINT64 GetBufferUsage() const { return bufferUsage; }
};
