#pragma once

class D3D12Sampler
{
public:
	D3D12_SAMPLER_DESC SamplerDesc;
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
};
