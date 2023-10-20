#pragma once

class D3D12DefaultBuffer : public D3D12Buffer
{
public:
	void CreateBuffer(ID3D12Device* device, const D3D12_RESOURCE_DESC* desc);
};