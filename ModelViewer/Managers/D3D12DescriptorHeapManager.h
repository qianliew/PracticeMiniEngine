#pragma once

class D3D12DescriptorHeapManager
{
private:
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_srvDescriptorSize;

public:
	D3D12DescriptorHeapManager(ComPtr<ID3D12Device>& device);

	void GetSRVHandle(D3D12SRV* handle, INT offset);

	void SetSRVs(ComPtr<ID3D12GraphicsCommandList>& commandList);
};