#pragma once

class D3D12View
{
protected:
	ID3D12Resource* pResource;
	ID3D12Resource* pCounterResource;

public:
	virtual ~D3D12View()
	{
		pResource = nullptr;
		pCounterResource = nullptr;
	}

	virtual void SetResource(ID3D12Resource* pResource) = 0;
	virtual void SetCounterResource(ID3D12Resource* pCounterResource) = 0;
	virtual void CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) = 0;
};

template<typename TParent, typename TDesc>
class TD3D12View : public D3D12View
{
protected:
	const TDesc Desc;

public:
	TD3D12View() = delete;
	TD3D12View(const TDesc& inDesc) :
		Desc(inDesc)
	{
		pResource = nullptr;
		pCounterResource = nullptr;
	}

	virtual ~TD3D12View()
	{

	}

	inline TDesc& GetDesc() { return Desc; }
	inline void SetResource(ID3D12Resource* inResource) override { pResource = inResource; }
	inline void SetCounterResource(ID3D12Resource* inResource) override { pCounterResource = inResource; }
};

class D3D12CBV final : public TD3D12View<D3D12CBV, D3D12_CONSTANT_BUFFER_VIEW_DESC>
{
public:
	D3D12CBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& inDesc)
		: TD3D12View(inDesc)
	{

	}

	void CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12SRV final : public TD3D12View<D3D12SRV, D3D12_SHADER_RESOURCE_VIEW_DESC>
{
public:
	D3D12SRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& inDesc)
		: TD3D12View(inDesc)
	{

	}

	void CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12RTV final : public TD3D12View<D3D12RTV, D3D12_RENDER_TARGET_VIEW_DESC>
{
public:
	D3D12RTV(const D3D12_RENDER_TARGET_VIEW_DESC& inDesc)
		: TD3D12View(inDesc)
	{

	}

	void CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12DSV final : public TD3D12View<D3D12DSV, D3D12_DEPTH_STENCIL_VIEW_DESC>
{
public:
	D3D12DSV(const D3D12_DEPTH_STENCIL_VIEW_DESC& inDesc)
		: TD3D12View(inDesc)
	{

	}

	void CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12UAV final : public TD3D12View<D3D12UAV, D3D12_UNORDERED_ACCESS_VIEW_DESC>
{
public:
	D3D12UAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC& inDesc)
		: TD3D12View(inDesc)
	{

	}

	void CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};
