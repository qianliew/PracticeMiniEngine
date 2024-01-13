#pragma once

#define GET_CONSTANT_BUFFER_SIZE(dataSize) (dataSize + UPLOAD_BUFFER_ALIGNMENT - 1) & ~(UPLOAD_BUFFER_ALIGNMENT - 1)

class D3D12Resource
{
protected:
	D3D12Buffer* pBuffer;

public:
	D3D12Resource();
	virtual ~D3D12Resource();

	void SetBuffer(D3D12Buffer* buffer);
	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) = 0;

	inline const D3D12_RESOURCE_DESC& GetResourceDesc() const { return pBuffer->GetResourceDesc(); }
	inline const ComPtr<ID3D12Resource>& GetResource() const { return pBuffer->GetResource(); }
};

template<typename TView, typename TViewDesc>
class TD3D12Resource : public D3D12Resource
{
protected:
	TView* view;
	TViewDesc viewDesc;

public:
	TD3D12Resource(const D3D12_RESOURCE_DESC& desc) :
		D3D12Resource(desc),
		view(nullptr)
	{

	}

	TD3D12Resource(const D3D12_RESOURCE_DESC& desc, const TViewDesc& viewDesc) :
		D3D12Resource(desc),
		viewDesc(viewDesc),
		view(nullptr)
	{

	}

	virtual ~TD3D12Resource() override
	{
		if (view != nullptr)
		{
			delete view;
		}
	}
};
