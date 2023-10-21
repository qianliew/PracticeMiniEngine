#include "stdafx.h"
#include "D3D12TextureBuffer.h"

D3D12TextureBuffer::~D3D12TextureBuffer()
{
    delete View;
}

void D3D12TextureBuffer::CreateViewDesc()
{
    if (View == nullptr)
    {
        View = new D3D12SRV();
        View->SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        View->SRVDesc.Format = ResourceDesc->Format;
        View->SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        View->SRVDesc.Texture2D.MipLevels = 1;
    }
}
