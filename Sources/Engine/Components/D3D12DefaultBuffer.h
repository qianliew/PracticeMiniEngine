#pragma once

class D3D12DefaultBuffer : public D3D12Buffer
{
public:
	void CreateBuffer(
		ID3D12Device* device,
		const D3D12_RESOURCE_DESC* desc,
		D3D12_RESOURCE_STATES state,
		const D3D12_CLEAR_VALUE* clearValue);
};