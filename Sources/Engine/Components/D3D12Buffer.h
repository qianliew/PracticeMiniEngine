#pragma once

class D3D12Buffer
{
protected:
	ComPtr<ID3D12Resource> pResource;
	void* pLocation;
	UINT64 bufferSize;
	UINT64 bufferUsage;

public:
	D3D12Buffer();
	virtual ~D3D12Buffer();

	inline ComPtr<ID3D12Resource>& GetResource() { return pResource; }
	inline void* GetLocation() { return pLocation; }
	inline const UINT64 GetBufferSize() const { return bufferSize; }
	inline const UINT64 GetBufferUsage() const { return bufferUsage; }
};
