#include "stdafx.h"
#include "D3D12CommandList.h"

D3D12CommandList::D3D12CommandList(ComPtr<ID3D12Device>& device, ComPtr<ID3D12CommandAllocator>& commandAllocator) :
    barrierIndex(0)
{
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(&commandList)));
}

D3D12CommandList::~D3D12CommandList()
{

}
