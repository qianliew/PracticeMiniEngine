#pragma once

class D3D12View
{
protected:
	D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle;
	ID3D12Resource* pResource;

public:
	virtual void SetResource(ID3D12Resource* pResource) = 0;
	virtual void CreateView(const ComPtr<ID3D12Device>& device) = 0;

	inline void SetCPUHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle) { CPUHandle = handle; }
};

template<typename TParent, typename TDesc>
class TD3D12View : public D3D12View
{
protected:
	TDesc Desc;

public:

	inline TDesc& GetDesc() { return Desc; }
	inline void SetResource(ID3D12Resource* inResource) override { pResource = inResource; }
};

class D3D12CBV final : public TD3D12View<D3D12CBV, D3D12_CONSTANT_BUFFER_VIEW_DESC>
{
public:
	void CreateView(const ComPtr<ID3D12Device>& device) override;
};

class D3D12SRV final : public TD3D12View<D3D12SRV, D3D12_SHADER_RESOURCE_VIEW_DESC>
{
public:
	void CreateView(const ComPtr<ID3D12Device>& device) override;
};

class D3D12DSV final : public TD3D12View<D3D12DSV, D3D12_DEPTH_STENCIL_VIEW_DESC>
{
public:
	void CreateView(const ComPtr<ID3D12Device>& device) override;
};
