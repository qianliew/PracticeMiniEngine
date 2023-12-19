#include "stdafx.h"
#include "FrustumCullingPass.h"
#include "CompiledShaders\FrustumCulling.hlsl.h"

FrustumCullingPass::FrustumCullingPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    AbstractRenderPass(device, sceneManager, viewManager)
{

}

void FrustumCullingPass::Setup(D3D12CommandList* pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{

}

void FrustumCullingPass::Execute(D3D12CommandList* pCommandList)
{

}
