#include "stdafx.h"
#include "D3D12TextureBuffer.h"

D3D12TextureBuffer::D3D12TextureBuffer(const D3D12_RESOURCE_DESC& desc) :
    TD3D12Resource(desc)
{

}

D3D12TextureBuffer::~D3D12TextureBuffer()
{

}

void D3D12TextureBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.Format = resourceDesc.Format;
    desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MostDetailedMip = 0;
    desc.Texture2D.MipLevels = 1;
    desc.Texture2D.PlaneSlice = 0;

    view = new D3D12SRV(desc);
    view->SetResource(resourceLocation.Resource.Get());
    view->CreateView(device, handle);
}

D3D12DepthStencilBuffer::D3D12DepthStencilBuffer(const D3D12_RESOURCE_DESC& desc) :
    TD3D12Resource(desc)
{

}

D3D12DepthStencilBuffer::~D3D12DepthStencilBuffer()
{

}


void D3D12DepthStencilBuffer::CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    D3D12_DEPTH_STENCIL_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    desc.Flags = D3D12_DSV_FLAG_NONE;
    desc.Texture2D.MipSlice = 0;

    view = new D3D12DSV(desc);
    view->SetResource(resourceLocation.Resource.Get());
    view->CreateView(device, handle);
}