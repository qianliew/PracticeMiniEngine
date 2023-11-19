#include "stdafx.h"
#include "D3D12CommandList.h"

D3D12CommandList::D3D12CommandList(std::shared_ptr<D3D12Device>& inDevice) :
    pDevice(inDevice),
    barrierIndex(0)
{
    ThrowIfFailed(pDevice->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        pDevice->GetCommandAllocator().Get(),
        nullptr,
        IID_PPV_ARGS(&pCommandList)));

    ThrowIfFailed(pCommandList->QueryInterface(IID_PPV_ARGS(&pDXRCommandList)));
}

D3D12CommandList::~D3D12CommandList()
{

}

void D3D12CommandList::ExecuteCommandList()
{
    ThrowIfFailed(pCommandList->Close());

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { pCommandList.Get() };
    pDevice->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}
