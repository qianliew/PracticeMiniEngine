#include "stdafx.h"
#include "IndirectDrawingPass.h"
#include "CompiledShaders/IndirectDrawing.hlsl.h"

IndirectDrawingPass::IndirectDrawingPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    AbstractRenderPass(device, sceneManager, viewManager)
{

}

void IndirectDrawingPass::Setup(D3D12CommandList* pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{
    // Describe and create the compute pipeline state object.
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = pRootSignature.Get();
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(g_pIndirectDrawing, ARRAYSIZE(g_pIndirectDrawing));

    ThrowIfFailed(pDevice->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pPipelineState.GetAddressOf())));
}

void IndirectDrawingPass::Execute(D3D12CommandList* pCommandList)
{
    // Set the pipeline state.
    pCommandList->SetPipelineState(pPipelineState.Get());

    // Bind resources for the indirect drawing.
    pSceneManager->SetIndirectDrawingResources(pCommandList);

    // Dispatch threads to filter objects.
    const UINT groupCountX = 1;
    const UINT groupCountY = 1;
    pCommandList->DispatchThreads(groupCountX, groupCountY, 1);
}
