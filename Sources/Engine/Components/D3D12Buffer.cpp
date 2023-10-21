#include "stdafx.h"
#include "D3D12Buffer.h"

D3D12Buffer::D3D12Buffer() :
    bufferSize(0),
    dataSize(0)
{
    ResourceLocation = new D3D12ResourceLocation();
}

D3D12Buffer::~D3D12Buffer()
{
    if (ResourceLocation->Resource != nullptr)
    {
        ResourceLocation->Resource->Unmap(0, nullptr);
    }
    delete ResourceLocation;
}

void* D3D12Buffer::GetStartLocation()
{
    return startLocation;
}

UINT D3D12Buffer::GetDataSize()
{
    return dataSize;
}