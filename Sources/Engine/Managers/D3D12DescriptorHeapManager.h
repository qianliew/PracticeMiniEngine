#pragma once

#define CONSTANT_BUFFER_VIEW_GLOBAL 0
#define CONSTANT_BUFFER_VIEW_PEROBJECT 1
#define SHADER_RESOURCE_VIEW 2
#define SAMPLER 3

#define UNDEFINED 100

#define FRAME_COUNT 2

class D3D12DescriptorHeapManager
{
private:
	std::map<UINT, ComPtr<ID3D12DescriptorHeap>> heapTable;
	std::map<UINT, UINT> sizeTable;

	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	ComPtr<ID3D12DescriptorHeap> dsvHeap;
	UINT rtvDescriptorSize;
	UINT dsvDescriptorSize;

public:
	D3D12DescriptorHeapManager(ComPtr<ID3D12Device>& device);
	~D3D12DescriptorHeapManager();

	D3D12_CPU_DESCRIPTOR_HANDLE GetHandle(UINT index, INT offset);
	void GetSamplerHandle(D3D12Sampler* const, INT offset);
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(INT offset);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle(INT offset);

	void SetCBVs(ComPtr<ID3D12GraphicsCommandList>&, UINT index, INT offset);
	void SetSRVs(ComPtr<ID3D12GraphicsCommandList>&, INT offset);
	void SetSamplers(ComPtr<ID3D12GraphicsCommandList>&, INT offset);
};