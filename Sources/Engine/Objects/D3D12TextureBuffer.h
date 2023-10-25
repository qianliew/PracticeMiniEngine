#pragma once

class D3D12TextureBuffer : public TD3D12Resource<D3D12SRV>
{
private:

public:
	D3D12TextureBuffer(UINT);
	~D3D12TextureBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};
