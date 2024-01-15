#pragma once

class D3D12ConstantBuffer : public TD3D12Resource<D3D12CBV, D3D12_CONSTANT_BUFFER_VIEW_DESC>
{
private:
	UINT64 size;

public:
	D3D12ConstantBuffer(UINT64 inSize);
	~D3D12ConstantBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
	void CopyData(void const* source, size_t size);
};
