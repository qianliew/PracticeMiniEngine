#include "stdafx.h"
#include "AbstractRenderPass.h"

AbstractRenderPass::AbstractRenderPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager) :
    pDevice(device),
    pSceneManager(sceneManager)
{
    pDepthStencil = new D3D12Texture(
        pSceneManager->GetCamera()->GetCameraWidth(),
        pSceneManager->GetCamera()->GetCameraHeight());
    pDepthStencil->CreateTexture(D3D12TextureType::DepthStencil);

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pDepthStencil->TextureBuffer.get(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue);
    pDepthStencil->TextureBuffer->CreateView(pDevice->GetDevice(), pDevice->GetDescriptorHeapManager()->GetDSVHandle(0));
}

AbstractRenderPass::~AbstractRenderPass()
{
    delete pDepthStencil;
}
