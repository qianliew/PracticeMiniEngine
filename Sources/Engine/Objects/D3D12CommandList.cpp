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

void D3D12CommandList::CopyBufferRegion(
    D3D12Resource* pDstResource,
    ID3D12Resource* pSrcResource,
    UINT64 NumBytes,
    UINT64 DstOffset,
    UINT64 SrcOffset)
{
    AddTransitionResourceBarriers(
        pDstResource->GetResource().Get(),
        pDstResource->GetResourceState(),
        D3D12_RESOURCE_STATE_COPY_DEST);
    FlushResourceBarriers();

    pCommandList->CopyBufferRegion(pDstResource->GetResource().Get(), DstOffset, pSrcResource, SrcOffset, NumBytes);

    AddTransitionResourceBarriers(
        pDstResource->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        pDstResource->GetResourceState());
    FlushResourceBarriers();
}

void D3D12CommandList::CopyResource(
    ID3D12Resource* pDstResource,
    ID3D12Resource* pSrcResource)
{
    pCommandList->CopyResource(pDstResource, pSrcResource);
}

void D3D12CommandList::CopyResource(
    D3D12Resource* pDstResource,
    ID3D12Resource* pSrcResource)
{
    AddTransitionResourceBarriers(
        pDstResource->GetResource().Get(),
        pDstResource->GetResourceState(),
        D3D12_RESOURCE_STATE_COPY_DEST);
    FlushResourceBarriers();

    pCommandList->CopyResource(pDstResource->GetResource().Get(), pSrcResource);

    AddTransitionResourceBarriers(
        pDstResource->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        pDstResource->GetResourceState());
    FlushResourceBarriers();
}

void D3D12CommandList::CopyResource(
    ID3D12Resource* pDstResource,
    D3D12Resource* pSrcResource)
{
    AddTransitionResourceBarriers(
        pSrcResource->GetResource().Get(),
        pSrcResource->GetResourceState(),
        D3D12_RESOURCE_STATE_COPY_SOURCE);
    FlushResourceBarriers();

    pCommandList->CopyResource(pDstResource, pSrcResource->GetResource().Get());

    AddTransitionResourceBarriers(
        pSrcResource->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        pSrcResource->GetResourceState());
    FlushResourceBarriers();
}

void D3D12CommandList::CopyResource(
    const D3D12Resource* pDstResource,
    const D3D12Resource* pSrcResource)
{
    AddTransitionResourceBarriers(
        pDstResource->GetResource().Get(),
        pDstResource->GetResourceState(),
        D3D12_RESOURCE_STATE_COPY_DEST);
    AddTransitionResourceBarriers(
        pSrcResource->GetResource().Get(),
        pSrcResource->GetResourceState(),
        D3D12_RESOURCE_STATE_COPY_SOURCE);
    FlushResourceBarriers();

    pCommandList->CopyResource(pDstResource->GetResource().Get(), pSrcResource->GetResource().Get());

    AddTransitionResourceBarriers(
        pDstResource->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        pDstResource->GetResourceState());
    AddTransitionResourceBarriers(
        pSrcResource->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE,
        pSrcResource->GetResourceState());
    FlushResourceBarriers();
}

void D3D12CommandList::CopyTextureBuffer(
    D3D12Resource* pDstResource,
    ID3D12Resource* pIntermediate,
    UINT64 IntermediateOffset,
    UINT FirstSubresource,
    UINT NumSubresources,
    const D3D12_SUBRESOURCE_DATA* pSrcData)
{
    AddTransitionResourceBarriers(
        pDstResource->GetResource().Get(),
        pDstResource->GetResourceState(),
        D3D12_RESOURCE_STATE_COPY_DEST);
    FlushResourceBarriers();

    UpdateSubresources(pCommandList.Get(), pDstResource->GetResource().Get(), pIntermediate,
        IntermediateOffset, FirstSubresource, NumSubresources, pSrcData);

    AddTransitionResourceBarriers(
        pDstResource->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        pDstResource->GetResourceState());
    FlushResourceBarriers();
}

void D3D12CommandList::CopyTexture(
    D3D12_TEXTURE_COPY_LOCATION* pDstResource,
    D3D12_TEXTURE_COPY_LOCATION* pSrcResource)
{
    pCommandList->CopyTextureRegion(pDstResource, 0, 0, 0, pSrcResource, nullptr);
}

void D3D12CommandList::AddTransitionResourceBarriers(
    ID3D12Resource* pResource,
    D3D12_RESOURCE_STATES stateBefore,
    D3D12_RESOURCE_STATES stateAfter)
{
    resourceBarriers[barrierIndex].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resourceBarriers[barrierIndex].Transition.pResource = pResource;
    resourceBarriers[barrierIndex].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    resourceBarriers[barrierIndex].Transition.StateBefore = stateBefore;
    resourceBarriers[barrierIndex++].Transition.StateAfter = stateAfter;
}

void D3D12CommandList::FlushResourceBarriers()
{
    pCommandList->ResourceBarrier(barrierIndex, &resourceBarriers[0]);
    barrierIndex = 0;
}
