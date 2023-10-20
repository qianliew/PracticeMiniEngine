#pragma once

class D3D12DescriptorHeapManager
{
private:
	UINT srvDescriptorSize;
	UINT samplerDescriptorSize;

	ComPtr<ID3D12DescriptorHeap> srvHeap;
	ComPtr<ID3D12DescriptorHeap> samplerHeap;

public:
	D3D12DescriptorHeapManager(ComPtr<ID3D12Device>& device);

	void GetSRVHandle(D3D12SRV*, INT offset);
	void GetSamplerHandle(D3D12Sampler* const, INT offset);

	void SetSRVs(ComPtr<ID3D12GraphicsCommandList>&);
	void SetSamplers(ComPtr<ID3D12GraphicsCommandList>&);
};