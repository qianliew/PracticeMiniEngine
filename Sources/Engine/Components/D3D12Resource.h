#pragma once

#define GET_CONSTANT_BUFFER_SIZE(dataSize) (dataSize + UPLOAD_BUFFER_ALIGNMENT - 1) & ~(UPLOAD_BUFFER_ALIGNMENT - 1)

class D3D12Resource
{
protected:
	D3D12ResourceLocation* resourceLocation;
	D3D12_RESOURCE_DESC* resourceDesc;
	UINT size;

public:
	D3D12Resource(UINT);
	virtual ~D3D12Resource();

	void SetResourceLoaction(const D3D12ResourceLocation*);
	void SetResourceDesc(D3D12_RESOURCE_DESC*);
	// TODO: Check nullptr.
	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) = 0;

	inline const D3D12ResourceLocation* GetResourceLocation() const { return resourceLocation; }
	inline const D3D12_RESOURCE_DESC* GetResourceDesc() const { return resourceDesc; }
	inline const UINT GetSize() const { return size; }

	inline ID3D12Resource* GetResource() const { return resourceLocation->Resource.Get(); }
};

template<typename TView>
class TD3D12Resource : public D3D12Resource
{
protected:
	TView view;

public:
	TD3D12Resource(UINT size)
		:D3D12Resource(size)
	{

	}

	~TD3D12Resource()
	{

	}
};
