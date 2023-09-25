#pragma once

class DescriptorHeapManager
{
private:
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_srvDescriptorSize;

public:
	DescriptorHeapManager(ComPtr<ID3D12Device>& device);

	void GetSRVHandle(View* handle, INT offset);

	void SetSRVs(ComPtr<ID3D12GraphicsCommandList>& commandList);
};