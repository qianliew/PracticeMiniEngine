#include "stdafx.h"
#include "D3D12Buffer.h"

D3D12Buffer::D3D12Buffer(const D3D12_RESOURCE_DESC& desc) :
    pLocation(nullptr),
    ResourceState(D3D12_RESOURCE_STATE_GENERIC_READ),
    resourceDesc(desc),
    bufferSize(0),
    bufferUsage(0)
{

}

D3D12Buffer::~D3D12Buffer()
{
    if (pResource != nullptr)
    {
        pResource->Unmap(0, nullptr);
    }
}
