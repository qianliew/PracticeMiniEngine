#pragma once

class D3D12Resource
{
protected:
	D3D12ResourceLocation* ResourceLocation;
	D3D12_RESOURCE_DESC* ResourceDesc;

public:
	void SetResourceLoaction(const D3D12ResourceLocation*);
	void SetResourceDesc(D3D12_RESOURCE_DESC*);
	virtual void CreateViewDesc() = 0;

	inline const D3D12ResourceLocation* GetResourceLocation() const { return ResourceLocation; }
	inline const D3D12_RESOURCE_DESC* GetResourceDesc() const { return ResourceDesc; }

	inline ID3D12Resource* GetResource() const { return ResourceLocation->Resource.Get(); }
};
