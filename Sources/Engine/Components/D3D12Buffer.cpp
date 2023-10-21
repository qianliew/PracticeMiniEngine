#include "stdafx.h"
#include "D3D12Buffer.h"

D3D12Buffer::D3D12Buffer()
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
    return m_startLocation;
}

UINT D3D12Buffer::GetDataSize()
{
    return m_dataSize;
}