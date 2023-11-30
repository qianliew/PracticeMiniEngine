#include "stdafx.h"
#include "BlitPass.h"

BlitPass::BlitPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    AbstractRenderPass(device, sceneManager, viewManager)
{

}

void BlitPass::Setup(D3D12CommandList*& pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ThrowIfFailed(D3DCompileFromFile(GetShaderPath(L"Blit.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSBlit", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    ThrowIfFailed(D3DCompileFromFile(GetShaderPath(L"Blit.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSBlit", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

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
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
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

void BlitPass::Execute(D3D12CommandList*& pCommandList)
{
    pCommandList->SetPipelineState(pPipelineState.Get());

    pViewManager->EmplaceRenderTarget(pCommandList, pViewManager->GetColorHandle(), D3D12TextureType::ShaderResource);
    pDevice->GetDescriptorHeapManager()->SetViews(
        pCommandList->GetCommandList(),
        SHADER_RESOURCE_VIEW_GLOBAL,
        pViewManager->GetTheSRVHandle(pViewManager->GetColorHandle()));
    pDevice->GetDescriptorHeapManager()->SetSamplers(pCommandList->GetCommandList(), 0);

    // Set camera relating state.
    pCommandList->SetViewports(pSceneManager->GetCamera()->GetViewport());
    pCommandList->SetScissorRects(pSceneManager->GetCamera()->GetScissorRect());

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pDevice->GetDescriptorHeapManager()->GetHandle(RENDER_TARGET_VIEW,
        pViewManager->GetFrameIndex());
    pCommandList->SetRenderTargets(1, &rtvHandle, nullptr);

    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    pCommandList->ClearColor(rtvHandle, clearColor);

    pSceneManager->DrawFullScreenMesh(pCommandList);
    pViewManager->EmplaceRenderTarget(pCommandList, pViewManager->GetColorHandle(), D3D12TextureType::RenderTarget);
}
