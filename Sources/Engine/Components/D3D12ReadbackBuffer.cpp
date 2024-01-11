#include "stdafx.h"
#include "D3D12ReadbackBuffer.h"

void D3D12ReadbackBuffer::CreateBuffer(
    ID3D12Device* device,
    UINT64 size,
    D3D12_RESOURCE_STATES state,
    const wchar_t* name)
{
    bufferSize = size;
    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        state,
        nullptr,
        IID_PPV_ARGS(pResource.GetAddressOf())));

    CD3DX12_RANGE readRange(0, 0);
    ThrowIfFailed(pResource->Map(0, &readRange, reinterpret_cast<void**>(&pLocation)));

    if (name)
    {
        pResource->SetName(name);
    }
}

void D3D12ReadbackBuffer::ReadbackData(void* destination)
{
    memcpy(destination, pLocation, bufferSize / 8);
}

void D3D12ReadbackBuffer::ReadbackData(void* destination, UINT size)
{
    memcpy(destination, pLocation, size);
}
