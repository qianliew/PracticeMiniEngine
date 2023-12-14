#include "stdafx.h"
#include "AbstractRenderPass.h"

AbstractRenderPass::AbstractRenderPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    pDevice(device),
    pSceneManager(sceneManager),
    pViewManager(viewManager)
{

}

AbstractRenderPass::~AbstractRenderPass()
{

}


void AbstractRenderPass::CopyBuffer(D3D12CommandList* pCommandList, const D3D12Resource* pDstResource, const D3D12Resource* pSrcResource)
{
    pCommandList->AddTransitionResourceBarriers(pDstResource->GetResource().Get(),
        pDstResource->GetResourceState(), D3D12_RESOURCE_STATE_COPY_DEST);
    pCommandList->AddTransitionResourceBarriers(pSrcResource->GetResource().Get(),
        pSrcResource->GetResourceState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    pCommandList->FlushResourceBarriers();

    pCommandList->CopyResource(pDstResource->GetResource().Get(), pSrcResource->GetResource().Get());

    pCommandList->AddTransitionResourceBarriers(pDstResource->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, pDstResource->GetResourceState());
    pCommandList->AddTransitionResourceBarriers(pSrcResource->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, pSrcResource->GetResourceState());
    pCommandList->FlushResourceBarriers();
}
