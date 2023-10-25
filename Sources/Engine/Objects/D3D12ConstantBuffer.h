#pragma once

class D3D12ConstantBuffer : public TD3D12Resource<D3D12CBV>
{
private:
	void* startLocation;

public:
	D3D12ConstantBuffer(UINT);
	~D3D12ConstantBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
	void CopyData(void const* source, size_t size);
	void SetStartLocation(void* location);
};
