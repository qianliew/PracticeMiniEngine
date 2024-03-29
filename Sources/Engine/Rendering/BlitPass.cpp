#include "stdafx.h"
#include "BlitPass.h"
#include "CompiledShaders/BlitVS.hlsl.h"
#include "CompiledShaders/BlitPS.hlsl.h"

BlitPass::BlitPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    AbstractRenderPass(device, sceneManager, viewManager)
{

}

void BlitPass::Setup(D3D12CommandList* pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
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
    psoDesc.VS = CD3DX12_SHADER_BYTECODE((void*)g_pBlitVS, ARRAYSIZE(g_pBlitVS));
    psoDesc.PS = CD3DX12_SHADER_BYTECODE((void*)g_pBlitPS, ARRAYSIZE(g_pBlitPS));
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
}

void BlitPass::Execute(D3D12CommandList* pCommandList)
{
    pCommandList->SetPipelineState(pPipelineState.Get());

    const UINT colorHandle = pViewManager->GetCurrentColorHandle();
    pViewManager->ConvertTextureType(pCommandList, colorHandle, D3D12TextureType::RenderTarget, D3D12TextureType::ShaderResource);
    pDevice->GetDescriptorHeapManager()->SetViews(
        pCommandList->GetCommandList(),
        SHADER_RESOURCE_VIEW_GLOBAL,
        (UINT)eRootIndex::ShaderResourceViewGlobal0,
        pViewManager->GetRTVSRVHandle(colorHandle));

    // Set camera relating state.
    pCommandList->SetViewports(pSceneManager->GetCamera()->GetViewport());
    pCommandList->SetScissorRects(pSceneManager->GetCamera()->GetScissorRect());

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pDevice->GetDescriptorHeapManager()->GetHandle(RENDER_TARGET_VIEW,
        pViewManager->GetFrameIndex());
    pCommandList->SetRenderTargets(1, &rtvHandle, nullptr);

    pSceneManager->DrawFullScreenMesh(pCommandList);
    pViewManager->ConvertTextureType(pCommandList, colorHandle, D3D12TextureType::RenderTarget, D3D12TextureType::RenderTarget);
}
