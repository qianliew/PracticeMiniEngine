#pragma once

class D3D12View
{
protected:
	D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle;

public:
	virtual void SetResource(ID3D12Resource* pResource) = 0;
	virtual void CreateView(const ComPtr<ID3D12Device>& device) = 0;

	inline void SetCPUHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle) { CPUHandle = handle; }
	// const D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return CPUHandle; }
};

template<typename TParent, typename TDesc>
class TD3D12View : public D3D12View
{
protected:
	TDesc Desc;

public:

	TDesc& GetDesc() { return Desc; }
};

class D3D12CBV : public TD3D12View<D3D12CBV, D3D12_CONSTANT_BUFFER_VIEW_DESC>
{
public:
	void CreateView(const ComPtr<ID3D12Device>& device) override;
	void SetResource(ID3D12Resource* pResource) override;
};

class D3D12SRV : public TD3D12View<D3D12SRV, D3D12_SHADER_RESOURCE_VIEW_DESC>
{
private:
	ID3D12Resource* resource;

public:
	void CreateView(const ComPtr<ID3D12Device>& device) override;
	void SetResource(ID3D12Resource* pResource) override;
};
