#include "stdafx.h"
#include "D3D12ReadbackBuffer.h"

D3D12ReadbackBuffer::D3D12ReadbackBuffer(const D3D12_RESOURCE_DESC& desc) :
    D3D12Buffer(desc)
{

}

D3D12ReadbackBuffer::~D3D12ReadbackBuffer()
{

}

void D3D12ReadbackBuffer::CreateBuffer(
    ID3D12Device* device,
    const wchar_t* name)
{
    ThrowIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        ResourceState,
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
