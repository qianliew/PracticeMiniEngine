#pragma once

template<typename TParent, typename TDesc>
class D3D12View
{
protected:
	TDesc Desc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;

public:
	virtual void CreateView(const ComPtr<ID3D12Device>& device) = 0;
	void SetHeapHandle(const ComPtr<ID3D12DescriptorHeap>& heap, UINT offset, UINT size)
	{
		CPUHandle = heap->GetCPUDescriptorHandleForHeapStart();
		CPUHandle.Offset(offset, size);
	}

	TDesc& GetDesc() { return Desc; }
	const D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return CPUHandle; }
};

class D3D12CBV : public D3D12View<D3D12CBV, D3D12_CONSTANT_BUFFER_VIEW_DESC>
{
public:
	void CreateView(const ComPtr<ID3D12Device>& device) override;
};

class D3D12SRV : public D3D12View<D3D12SRV, D3D12_SHADER_RESOURCE_VIEW_DESC>
{
private:
	ID3D12Resource* resource;

public:
	void CreateView(const ComPtr<ID3D12Device>& device) override;
	void SetResource(ID3D12Resource* pResource);
};
