#pragma once

struct D3D12ResourceLocation
{
	ComPtr<ID3D12Resource> Resource;
};

class D3D12Buffer
{
protected:
	void* startLocation;
	UINT64 bufferSize;
	UINT64 bufferUsage;

public:
	D3D12ResourceLocation ResourceLocation;
	D3D12Buffer();
	virtual ~D3D12Buffer();

	inline void* GetStartLocation() { return startLocation; }
	inline const UINT64 GetBufferSize() const { return bufferSize; }
	inline const UINT64 GetBufferUsage() const { return bufferUsage; }
};
