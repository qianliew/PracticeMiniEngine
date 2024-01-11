#include "stdafx.h"
#include "D3D12ShaderResourceBuffer.h"

D3D12ShaderResourceBuffer::D3D12ShaderResourceBuffer(
    const D3D12_RESOURCE_DESC& desc, 
    const D3D12_SHADER_RESOURCE_VIEW_DESC& viewDesc) :
    TD3D12Resource(desc, viewDesc)
{
    view = new D3D12SRV(viewDesc);
}

D3D12ShaderResourceBuffer::~D3D12ShaderResourceBuffer()
{

}

void D3D12ShaderResourceBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    view->SetResource(resourceLocation.Resource.Get());
    view->CreateView(device, handle);
}

D3D12RenderTargetBuffer::D3D12RenderTargetBuffer(
    const D3D12_RESOURCE_DESC& desc,
    const D3D12_RENDER_TARGET_VIEW_DESC& viewDesc) :
    TD3D12Resource(desc, viewDesc)
{
    view = new D3D12RTV(viewDesc);
}

D3D12RenderTargetBuffer::~D3D12RenderTargetBuffer()
{

}

void D3D12RenderTargetBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    view->SetResource(resourceLocation.Resource.Get());
    view->CreateView(device, handle);
}

D3D12DepthStencilBuffer::D3D12DepthStencilBuffer(
    const D3D12_RESOURCE_DESC& desc,
    const D3D12_DEPTH_STENCIL_VIEW_DESC& viewDesc) :
    TD3D12Resource(desc, viewDesc)
{

}

D3D12DepthStencilBuffer::~D3D12DepthStencilBuffer()
{

}

void D3D12DepthStencilBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    view = new D3D12DSV(viewDesc);
    view->SetResource(resourceLocation.Resource.Get());
    view->CreateView(device, handle);
}

D3D12UnorderedAccessBuffer::D3D12UnorderedAccessBuffer(
    const D3D12_RESOURCE_DESC& desc,
    const D3D12_UNORDERED_ACCESS_VIEW_DESC& viewDesc) :
    TD3D12Resource(desc, viewDesc)
{
    view = new D3D12UAV(viewDesc);
}

D3D12UnorderedAccessBuffer::~D3D12UnorderedAccessBuffer()
{

}

void D3D12UnorderedAccessBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    view->SetResource(resourceLocation.Resource.Get());
    view->CreateView(device, handle);
}

void D3D12UnorderedAccessBuffer::SetCounterResource(ID3D12Resource* pCounterResource)
{
    view->SetCounterResource(pCounterResource == nullptr ? resourceLocation.Resource.Get() : pCounterResource);
}

void D3D12UnorderedAccessBuffer::RemoveCounterResource()
{
    view->SetCounterResource(nullptr);
}
