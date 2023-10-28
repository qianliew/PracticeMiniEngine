#pragma once

#define GET_CONSTANT_BUFFER_SIZE(dataSize) (dataSize + UPLOAD_BUFFER_ALIGNMENT - 1) & ~(UPLOAD_BUFFER_ALIGNMENT - 1)

class D3D12Resource
{
protected:
	D3D12ResourceLocation resourceLocation;
	const D3D12_RESOURCE_DESC resourceDesc;

public:
	D3D12Resource() = delete;
	D3D12Resource(const D3D12_RESOURCE_DESC&);
	virtual ~D3D12Resource();

	void SetResourceLoaction(const D3D12ResourceLocation&);
	// TODO: Check nullptr.
	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) = 0;

	inline const D3D12_RESOURCE_DESC GetResourceDesc() const { return resourceDesc; }
	inline ID3D12Resource* GetResource() const { return resourceLocation.Resource.Get(); }
};

template<typename TView>
class TD3D12Resource : public D3D12Resource
{
protected:
	TView* view;

public:
	TD3D12Resource(const D3D12_RESOURCE_DESC& desc)
		: D3D12Resource(desc)
	{

	}

	virtual ~TD3D12Resource() override
	{
		delete view;
	}
};
