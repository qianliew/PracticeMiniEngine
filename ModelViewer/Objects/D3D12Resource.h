#pragma once

class D3D12Resource
{
private:

protected:

public:
	D3D12Resource();
	~D3D12Resource();

	D3D12ResourceLocation* ResourceLocation;
	D3D12_RESOURCE_DESC* ResourceDesc;
};
