#pragma once

class D3D12ReadbackBuffer : public D3D12Buffer
{
public:
	D3D12ReadbackBuffer(const D3D12_RESOURCE_DESC& desc);
	virtual ~D3D12ReadbackBuffer();

	void CreateBuffer(
		ID3D12Device* device,
		const wchar_t* name,
		const D3D12_CLEAR_VALUE* clearValue,
		const D3D12_RESOURCE_STATES state) override;

	void ReadbackData(void* destination);
	void ReadbackData(void* destination, UINT size);
};
