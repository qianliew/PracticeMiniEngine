#pragma once

#define CONSTANT_BUFFER_VIEW_GLOBAL 0
#define CONSTANT_BUFFER_VIEW_PEROBJECT 1
#define SHADER_RESOURCE_VIEW 2
#define SAMPLER 3

class D3D12DescriptorHeapManager
{
private:
	UINT cbvDescriptorSize;
	UINT srvDescriptorSize;
	UINT samplerDescriptorSize;

	std::map<UINT, ComPtr<ID3D12DescriptorHeap>> heaps;
	ComPtr<ID3D12DescriptorHeap> srvHeap;
	ComPtr<ID3D12DescriptorHeap> samplerHeap;

public:
	D3D12DescriptorHeapManager(ComPtr<ID3D12Device>& device);
	~D3D12DescriptorHeapManager();

	void GetCBVHandle(D3D12CBV*, UINT index, INT offset);
	void GetSRVHandle(D3D12SRV*, INT offset);
	void GetSamplerHandle(D3D12Sampler* const, INT offset);

	void SetCBVs(ComPtr<ID3D12GraphicsCommandList>&, UINT index, INT offset);
	void SetSRVs(ComPtr<ID3D12GraphicsCommandList>&);
	void SetSamplers(ComPtr<ID3D12GraphicsCommandList>&);
};