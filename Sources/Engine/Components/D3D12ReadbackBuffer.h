#pragma once

class D3D12ReadbackBuffer : public D3D12Buffer
{
private:
	D3D12_RESOURCE_DESC resourceDesc;

public:
	void CreateBuffer(
		ID3D12Device* device,
		UINT64 size,
		D3D12_RESOURCE_STATES state,
		const wchar_t* name);
	void ReadbackData(void* destination);
};
