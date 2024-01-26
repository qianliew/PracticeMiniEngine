#include "stdafx.h"
#include "GBufferPass.h"
#include "CompiledShaders/GBufferVS.hlsl.h"
#include "CompiledShaders/GBufferPS.hlsl.h"

GBufferPass::GBufferPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    AbstractRenderPass(device, sceneManager, viewManager)
{

}

void GBufferPass::Setup(D3D12CommandList* pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{
    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = pRootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(g_pGBufferVS, ARRAYSIZE(g_pGBufferVS));
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(g_pGBufferPS, ARRAYSIZE(g_pGBufferPS));
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = pViewManager->GetGBufferCount();
    for (UINT i = 0; i < psoDesc.NumRenderTargets; i++)
    {
        psoDesc.RTVFormats[i] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    }
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(pDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pPipelineState.GetAddressOf())));
}

void GBufferPass::Execute(D3D12CommandList* pCommandList)
{
    // Set the pipeline state.
    pCommandList->SetPipelineState(pPipelineState.Get());

    // Set camera relating states.
    pCommandList->SetViewports(pSceneManager->GetCamera()->GetViewport());
    pCommandList->SetScissorRects(pSceneManager->GetCamera()->GetScissorRect());

    // Set the rtv and the dsv.
    UINT const gBufferHandle = pViewManager->GetGBufferHandle(0);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        pDevice->GetDescriptorHeapManager()->GetHandle(RENDER_TARGET_VIEW, gBufferHandle);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
        pDevice->GetDescriptorHeapManager()->GetHandle(DEPTH_STENCIL_VIEW, 0);
    pCommandList->SetRenderTargets(pViewManager->GetGBufferCount(), &rtvHandle, &dsvHandle);

    // Clear the rtv and the dsv.
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for (UINT i = 0; i < pViewManager->GetGBufferCount(); i++)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle =
            pDevice->GetDescriptorHeapManager()->GetHandle(RENDER_TARGET_VIEW, pViewManager->GetGBufferHandle(i));
        pCommandList->ClearColor(handle, clearColor);
    }
    pCommandList->ClearDepth(dsvHandle);

    // pSceneManager->DrawObjects(pCommandList);
    pSceneManager->DrawObjectsIndirectly(pCommandList);
}
