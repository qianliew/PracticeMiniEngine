#pragma once

struct D3D12ResourceLocation
{
	ComPtr<ID3D12Resource> Resource;
};

class D3D12Buffer
{
protected:
	void* startLocation;
	UINT bufferSize;

public:
	D3D12ResourceLocation* ResourceLocation;
	D3D12Buffer();
	~D3D12Buffer();

	void* GetStartLocation();
	UINT GetBufferSize();
};
