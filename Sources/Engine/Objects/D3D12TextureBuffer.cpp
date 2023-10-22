#include "stdafx.h"
#include "D3D12TextureBuffer.h"

D3D12TextureBuffer::D3D12TextureBuffer(UINT size) :
    D3D12Resource(size)
{
    View = new D3D12SRV();
}

D3D12TextureBuffer::~D3D12TextureBuffer()
{
    delete View;
}

void D3D12TextureBuffer::CreateViewDesc()
{
    View->SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    View->SRVDesc.Format = resourceDesc->Format;
    View->SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    View->SRVDesc.Texture2D.MipLevels = 1;
}
