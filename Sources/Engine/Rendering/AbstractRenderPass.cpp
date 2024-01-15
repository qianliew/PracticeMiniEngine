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
    pCommandList->CopyResource(pDstResource, pSrcResource);
}
