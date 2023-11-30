#include "stdafx.h"
#include "DrawObjecstPass.h"

DrawObjectsPass::DrawObjectsPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    AbstractRenderPass(device, sceneManager, viewManager)
{

}

void DrawObjectsPass::Setup(D3D12CommandList*& pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ThrowIfFailed(D3DCompileFromFile(GetShaderPath(L"Lit.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    ThrowIfFailed(D3DCompileFromFile(GetShaderPath(L"Lit.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

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
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
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

void DrawObjectsPass::Execute(D3D12CommandList*& pCommandList)
{
    // Set the pipeline state.
    pCommandList->SetPipelineState(pPipelineState.Get());

    // Set the global views.
    pCommandList->SetRootConstantBufferView(CONSTANT_BUFFER_VIEW_GLOBAL,
        pDevice->GetBufferManager()->GetGlobalConstantBuffer()->GetResource()->GetGPUVirtualAddress());

    // Set camera relating states.
    pCommandList->SetViewports(pSceneManager->GetCamera()->GetViewport());
    pCommandList->SetScissorRects(pSceneManager->GetCamera()->GetScissorRect());

    // Set the rtv and the dsv.
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        pDevice->GetDescriptorHeapManager()->GetHandle(RENDER_TARGET_VIEW, pViewManager->GetColorHandle());
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
        pDevice->GetDescriptorHeapManager()->GetHandle(DEPTH_STENCIL_VIEW, 0);
    pCommandList->SetRenderTargets(1, &rtvHandle, &dsvHandle);

    // Clear the rtv and the dsv.
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    pCommandList->ClearColor(rtvHandle, clearColor);
    pCommandList->ClearDepth(dsvHandle);

    pSceneManager->DrawObjects(pCommandList);
}
