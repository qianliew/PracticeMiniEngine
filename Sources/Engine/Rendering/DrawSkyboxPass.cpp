#include "stdafx.h"
#include "DrawSkyboxPass.h"

DrawSkyboxPass::DrawSkyboxPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager) :
    AbstractRenderPass(device, sceneManager)
{

}

void DrawSkyboxPass::Setup(D3D12CommandList*&, ComPtr<ID3D12RootSignature>&)
{

}

void DrawSkyboxPass::Execute(D3D12CommandList*&, UINT)
{

}
