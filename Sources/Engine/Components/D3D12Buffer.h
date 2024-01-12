#pragma once

class D3D12Buffer
{
protected:
	ComPtr<ID3D12Resource> pResource;
	void* pLocation;

	const D3D12_RESOURCE_DESC resourceDesc;
	UINT64 bufferSize;
	UINT64 bufferUsage;

public:
	D3D12Buffer() = delete;
	D3D12Buffer(const D3D12Buffer&) = delete;
	D3D12Buffer& operator= (const D3D12Buffer&) = delete;
	D3D12Buffer(const D3D12_RESOURCE_DESC& desc);
	virtual ~D3D12Buffer();

	D3D12_RESOURCE_STATES ResourceState;

	inline ComPtr<ID3D12Resource>& GetResource() { return pResource; }
	inline void* GetLocation() { return pLocation; }
	inline const D3D12_RESOURCE_DESC& GetResourceDesc() const { return resourceDesc; }
	inline const UINT64 GetBufferSize() const { return bufferSize; }
	inline const UINT64 GetBufferUsage() const { return bufferUsage; }
};
