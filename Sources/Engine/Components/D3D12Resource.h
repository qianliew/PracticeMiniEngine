#pragma once

#define GET_CONSTANT_BUFFER_SIZE(dataSize) (dataSize + UPLOAD_BUFFER_ALIGNMENT - 1) & ~(UPLOAD_BUFFER_ALIGNMENT - 1)

class D3D12Resource
{
protected:
	D3D12_RESOURCE_STATES resourceState;
	D3D12ResourceLocation resourceLocation;
	const D3D12_RESOURCE_DESC resourceDesc;

public:
	D3D12Resource() = delete;
	D3D12Resource(const D3D12_RESOURCE_DESC&);
	virtual ~D3D12Resource();

	void SetResourceLoaction(const ComPtr<ID3D12Resource>&);
	// TODO: Check nullptr.
	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) = 0;

	inline const D3D12_RESOURCE_DESC& GetResourceDesc() const { return resourceDesc; }
	inline const ComPtr<ID3D12Resource>& GetResource() const { return resourceLocation.Resource; }
	inline const D3D12_RESOURCE_STATES GetResourceState() const { return resourceState; }
	inline void SetResourceState(D3D12_RESOURCE_STATES state) { resourceState = state; }
};

template<typename TView, typename TViewDesc>
class TD3D12Resource : public D3D12Resource
{
protected:
	TView* view;
	TViewDesc viewDesc;

public:
	TD3D12Resource(const D3D12_RESOURCE_DESC& desc) :
		D3D12Resource(desc)
	{

	}

	TD3D12Resource(const D3D12_RESOURCE_DESC& desc, const TViewDesc& viewDesc) :
		D3D12Resource(desc),
		viewDesc(viewDesc)
	{

	}

	virtual ~TD3D12Resource() override
	{
		delete view;
	}
};
