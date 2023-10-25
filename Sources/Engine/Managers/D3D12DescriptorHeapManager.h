#pragma once

#define CONSTANT_BUFFER_VIEW_GLOBAL 0
#define CONSTANT_BUFFER_VIEW_PEROBJECT 1
#define SHADER_RESOURCE_VIEW 2
#define SAMPLER 3

#define UNDEFINED 100

class D3D12DescriptorHeapManager
{
private:
	std::map<UINT, ComPtr<ID3D12DescriptorHeap>> heapTable;
	std::map<UINT, UINT> sizeTable;

public:
	D3D12DescriptorHeapManager(ComPtr<ID3D12Device>& device);
	~D3D12DescriptorHeapManager();

	D3D12_CPU_DESCRIPTOR_HANDLE GetHandle(UINT index, INT offset);
	void GetSamplerHandle(D3D12Sampler* const, INT offset);

	void SetCBVs(ComPtr<ID3D12GraphicsCommandList>&, UINT index, INT offset);
	void SetSRVs(ComPtr<ID3D12GraphicsCommandList>&);
	void SetSamplers(ComPtr<ID3D12GraphicsCommandList>&);
};