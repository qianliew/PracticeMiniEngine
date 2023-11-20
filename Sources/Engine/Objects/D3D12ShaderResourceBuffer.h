#pragma once

class D3D12ShaderResourceBuffer : public TD3D12Resource<D3D12SRV, D3D12_SHADER_RESOURCE_VIEW_DESC>
{
private:

public:
	D3D12ShaderResourceBuffer(const D3D12_RESOURCE_DESC&, const D3D12_SHADER_RESOURCE_VIEW_DESC&);
	~D3D12ShaderResourceBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12RenderTargetBuffer : public TD3D12Resource<D3D12RTV, D3D12_RENDER_TARGET_VIEW_DESC>
{
private:

public:
	D3D12RenderTargetBuffer(const D3D12_RESOURCE_DESC&, const D3D12_RENDER_TARGET_VIEW_DESC&);
	~D3D12RenderTargetBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12DepthStencilBuffer : public TD3D12Resource<D3D12DSV, D3D12_DEPTH_STENCIL_VIEW_DESC>
{
private:

public:
	D3D12DepthStencilBuffer(const D3D12_RESOURCE_DESC&, const D3D12_DEPTH_STENCIL_VIEW_DESC&);
	~D3D12DepthStencilBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};

class D3D12UnorderedAccessBuffer : public TD3D12Resource<D3D12UAV, D3D12_UNORDERED_ACCESS_VIEW_DESC>
{
private:

public:
	D3D12UnorderedAccessBuffer(const D3D12_RESOURCE_DESC&, const D3D12_UNORDERED_ACCESS_VIEW_DESC&);
	~D3D12UnorderedAccessBuffer();

	virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
};
