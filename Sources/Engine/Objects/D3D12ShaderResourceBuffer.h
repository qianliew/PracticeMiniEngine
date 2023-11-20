#pragma once

class D3D12ShaderResourceBuffer : public TD3D12Resource<D3D12SRV>
{
private:
	const D3D12_SRV_DIMENSION srvDimension;

public:
	D3D12ShaderResourceBuffer(const D3D12_RESOURCE_DESC&, const D3D12_SRV_DIMENSION);
	~D3D12ShaderResourceBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12RenderTargetBuffer : public TD3D12Resource<D3D12RTV>
{
private:

public:
	D3D12RenderTargetBuffer(const D3D12_RESOURCE_DESC&);
	~D3D12RenderTargetBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12DepthStencilBuffer : public TD3D12Resource<D3D12DSV>
{
private:

public:
	D3D12DepthStencilBuffer(const D3D12_RESOURCE_DESC&);
	~D3D12DepthStencilBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12UnorderedAccessBuffer : public TD3D12Resource<D3D12UAV>
{
private:

public:
	D3D12UnorderedAccessBuffer(const D3D12_RESOURCE_DESC&);
	~D3D12UnorderedAccessBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};
