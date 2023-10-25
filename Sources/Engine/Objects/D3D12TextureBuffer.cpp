#include "stdafx.h"
#include "D3D12TextureBuffer.h"

D3D12TextureBuffer::D3D12TextureBuffer(UINT size) :
    TD3D12Resource(size)
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
    view.GetDesc().Format = resourceDesc->Format;
    view.GetDesc().ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    view.GetDesc().Texture2D.MostDetailedMip = 0;
    view.GetDesc().Texture2D.MipLevels = 1;
    view.GetDesc().Texture2D.PlaneSlice = 0;

    view.CreateView(device);
}
