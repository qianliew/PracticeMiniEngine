#pragma once

class D3D12DefaultBuffer : public D3D12Buffer
{
public:
	D3D12DefaultBuffer(const D3D12_RESOURCE_DESC& desc);
	virtual ~D3D12DefaultBuffer();

	void CreateBuffer(
		ID3D12Device* device,
		const wchar_t* name,
		const D3D12_CLEAR_VALUE* clearValue,
		const D3D12_RESOURCE_STATES state) override;
};