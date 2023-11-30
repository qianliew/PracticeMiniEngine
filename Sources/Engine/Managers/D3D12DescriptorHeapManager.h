#pragma once

#define CONSTANT_BUFFER_VIEW_GLOBAL 0
#define CONSTANT_BUFFER_VIEW_PEROBJECT 1
#define SHADER_RESOURCE_VIEW_GLOBAL 2
#define SHADER_RESOURCE_VIEW_PEROBJECT 3
#define UNORDERED_ACCESS_VIEW 4
#define SAMPLER 5
#define RENDER_TARGET_VIEW 6
#define DEPTH_STENCIL_VIEW 7

class D3D12DescriptorHeapManager
{
private:
	std::map<UINT, ComPtr<ID3D12DescriptorHeap>> heapTable;
	std::map<UINT, UINT> sizeTable;

public:
	D3D12DescriptorHeapManager(ComPtr<ID3D12Device>& device, BOOL isDXR);
	~D3D12DescriptorHeapManager();

	D3D12_CPU_DESCRIPTOR_HANDLE GetHandle(UINT index, INT offset);

	void SetViews(ComPtr<ID3D12GraphicsCommandList>&, UINT index, INT offset);
	void SetComputeViews(ComPtr<ID3D12GraphicsCommandList>&, UINT index, UINT rootIndex, INT offset);
	void SetSRVs(ComPtr<ID3D12GraphicsCommandList>&, INT offset);
	void SetSamplers(ComPtr<ID3D12GraphicsCommandList>&, INT offset);
};