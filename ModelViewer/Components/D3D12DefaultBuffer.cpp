#include "stdafx.h"
#include "D3D12DefaultBuffer.h"

void D3D12DefaultBuffer::CreateBuffer(ID3D12Device* device, const D3D12_RESOURCE_DESC* desc)
{
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(ResourceLocation->Resource.GetAddressOf())));
}