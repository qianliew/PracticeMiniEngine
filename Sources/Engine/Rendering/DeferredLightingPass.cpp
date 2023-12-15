#include "stdafx.h"
#include "DeferredLightingPass.h"

DeferredLightingPass::DeferredLightingPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    AbstractRenderPass(device, sceneManager, viewManager)
{

}

void DeferredLightingPass::Setup(D3D12CommandList* pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{
    ComPtr<ID3DBlob> computeShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ThrowIfFailed(D3DCompileFromFile(GetShaderPath(
        L"DeferredLighting.hlsl").c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "CSMain",
        "cs_5_0",
        compileFlags,
        0,
        &computeShader,
        nullptr));

    // Describe and create the compute pipeline state object.
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = pRootSignature.Get();
    psoDesc.CS = CD3DX12_SHADER_BYTECODE(computeShader.Get());

    ThrowIfFailed(pDevice->GetDevice()->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pPipelineState.GetAddressOf())));
}

void DeferredLightingPass::Execute(D3D12CommandList* pCommandList)
{
    // Set the pipeline state.
    pCommandList->SetPipelineState(pPipelineState.Get());

    // Bind the UAV heap for the output.
    pDevice->GetDescriptorHeapManager()->SetComputeViews(
        pCommandList->GetCommandList(),
        UNORDERED_ACCESS_VIEW,
        (UINT)eRootIndex::UnorderedAccessViewGlobal,
        0);

    // Dispatch threads to shade the lighting.
    pCommandList->DispatchThreads(5, 5, 1);

    const D3D12Resource* pColorResource = pViewManager->GetCurrentBuffer(pViewManager->GetCurrentColorHandle());
    const D3D12Resource* pOutputResource = pViewManager->GetRayTracingOutput();
    CopyBuffer(pCommandList, pColorResource, pOutputResource);
}
