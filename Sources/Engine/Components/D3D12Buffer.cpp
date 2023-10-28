#include "stdafx.h"
#include "D3D12Buffer.h"

D3D12Buffer::D3D12Buffer() :
    bufferSize(0)
{

}

D3D12Buffer::~D3D12Buffer()
{
    if (ResourceLocation.Resource != nullptr)
    {
        ResourceLocation.Resource->Unmap(0, nullptr);
    }
}
