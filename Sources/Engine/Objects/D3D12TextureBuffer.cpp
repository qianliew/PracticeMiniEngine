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
    view.SetResource(resourceLocation->Resource.Get());
    view.SetCPUHandle(handle);

    view.GetDesc().Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    view.GetDesc().Format = resourceDesc.Format;
    view.GetDesc().ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    view.GetDesc().Texture2D.MostDetailedMip = 0;
    view.GetDesc().Texture2D.MipLevels = 1;
    view.GetDesc().Texture2D.PlaneSlice = 0;

    view.CreateView(device);
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
    view.SetResource(resourceLocation->Resource.Get());
    view.SetCPUHandle(handle);

    view.GetDesc().Format = DXGI_FORMAT_D32_FLOAT;
    view.GetDesc().ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    view.GetDesc().Flags = D3D12_DSV_FLAG_NONE;

    view.CreateView(device);
}
