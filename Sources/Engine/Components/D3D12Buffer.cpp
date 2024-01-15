#include "stdafx.h"
#include "D3D12Buffer.h"

D3D12Buffer::D3D12Buffer(const D3D12_RESOURCE_DESC& desc) :
    pLocation(nullptr),
    resourceState(D3D12_RESOURCE_STATE_GENERIC_READ),
    resourceDesc(desc),
    bufferUsage(0)
{

}

D3D12Buffer::~D3D12Buffer()
{

}
