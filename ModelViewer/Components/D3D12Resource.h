#pragma once

class D3D12Resource
{
public:
	D3D12Resource();
	~D3D12Resource();

	virtual void CreateView() = 0;

	D3D12ResourceLocation* ResourceLocation;
	D3D12_RESOURCE_DESC* ResourceDesc;
};
