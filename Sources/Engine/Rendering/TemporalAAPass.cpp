#include "stdafx.h"
#include "TemporalAAPass.h"
#include "CompiledShaders/TemporalAAVS.hlsl.h"
#include "CompiledShaders/TemporalAAPS.hlsl.h"

TemporalAAPass::TemporalAAPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    AbstractRenderPass(device, sceneManager, viewManager)
{

}

void TemporalAAPass::Setup(D3D12CommandList* pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = pRootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(g_pTemporalAAVS, ARRAYSIZE(g_pTemporalAAVS));
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(g_pTemporalAAPS, ARRAYSIZE(g_pTemporalAAPS));
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(pDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pPipelineState.GetAddressOf())));

    // Create a render target for TAA history.
    taaHistoryHandle = pViewManager->CreateRenderTargetView(DXGI_FORMAT_R8G8B8A8_UNORM);
}

void TemporalAAPass::Execute(D3D12CommandList* pCommandList)
{
    pCommandList->SetPipelineState(pPipelineState.Get());

    // Set the color buffer and the TAA history to the SRVs.
    const UINT colorHandle = pViewManager->GetCurrentColorHandle();
    const UINT taaHandle = pViewManager->GetNextColorHandle();
    const UINT depthHandle = 0;

    pViewManager->ConvertTextureType(pCommandList, colorHandle, D3D12TextureType::RenderTarget, D3D12TextureType::ShaderResource);
    pViewManager->ConvertTextureType(pCommandList, taaHistoryHandle, D3D12TextureType::RenderTarget, D3D12TextureType::ShaderResource);
    pViewManager->ConvertTextureType(pCommandList, depthHandle, D3D12TextureType::DepthStencil, D3D12TextureType::ShaderResource);
    pDevice->GetDescriptorHeapManager()->SetViews(
        pCommandList->GetCommandList(),
        SHADER_RESOURCE_VIEW_GLOBAL,
        (UINT)eRootIndex::ShaderResourceViewGlobal0,
        pViewManager->GetRTVSRVHandle(colorHandle));
    pDevice->GetDescriptorHeapManager()->SetViews(
        pCommandList->GetCommandList(),
        SHADER_RESOURCE_VIEW_GLOBAL,
        (UINT)eRootIndex::ShaderResourceViewGlobal1,
        pViewManager->GetRTVSRVHandle(taaHistoryHandle));
    pDevice->GetDescriptorHeapManager()->SetViews(
        pCommandList->GetCommandList(),
        SHADER_RESOURCE_VIEW_GLOBAL,
        (UINT)eRootIndex::ShaderResourceViewGlobal2,
        pViewManager->GetDSVSRVHandle(pViewManager->GetCurrentDSVHandle()));

    // Set the TAA handle to the render targert, and draw. 
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        pDevice->GetDescriptorHeapManager()->GetHandle(RENDER_TARGET_VIEW, taaHandle);
    pCommandList->SetRenderTargets(1, &rtvHandle, nullptr);

    // Set camera relating state.
    pCommandList->SetViewports(pSceneManager->GetCamera()->GetViewport());
    pCommandList->SetScissorRects(pSceneManager->GetCamera()->GetScissorRect());

    pSceneManager->DrawFullScreenMesh(pCommandList);
    pViewManager->ConvertTextureType(pCommandList, colorHandle, D3D12TextureType::RenderTarget, D3D12TextureType::RenderTarget);
    pViewManager->ConvertTextureType(pCommandList, taaHistoryHandle, D3D12TextureType::RenderTarget, D3D12TextureType::RenderTarget);
    pViewManager->ConvertTextureType(pCommandList, depthHandle, D3D12TextureType::DepthStencil, D3D12TextureType::DepthStencil);

    // Copy the current TAA buffer to the history.
    D3D12Resource* pTAAHistoryResource = pViewManager->GetCurrentRTVBuffer(taaHistoryHandle);
    D3D12Resource* pTAAResource = pViewManager->GetCurrentRTVBuffer(taaHandle);

    pCommandList->CopyResource(pTAAHistoryResource, pTAAResource);
}
