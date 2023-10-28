#include "stdafx.h"
#include "MiniEngine.h"

using namespace Microsoft::WRL;

MiniEngine::MiniEngine(UINT width, UINT height, std::wstring name) :
    Window(width, height, name),
    frameIndex(0)
{
    WCHAR path[512];
    GetAssetsPath(path, _countof(path));
    assetsPath = path;
}

MiniEngine::~MiniEngine()
{
    delete pCommandList;
}

// Helper function for resolving the full path of assets.
std::wstring MiniEngine::GetAssetFullPath(LPCWSTR assetName)
{
    return assetsPath + assetName;
}

void MiniEngine::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void MiniEngine::LoadPipeline()
{
    pDevice = std::make_shared<D3D12Device>();
    pDevice->CreateDevice();

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(pDevice->GetFactory()->CreateSwapChainForHwnd(
        pDevice->GetCommandQueue().Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(pDevice->GetFactory()->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain1.As(&pSwapChain));
    frameIndex = pSwapChain->GetCurrentBackBufferIndex();

    pDevice->CreateDescriptorHeapManager();
    pDevice->CreateBufferManager();

    // Create frame resources.
    // Create a RTV for each frame.
    for (UINT n = 0; n < FRAME_COUNT; n++)
    {
        ThrowIfFailed(pSwapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
        pDevice->GetDevice()->CreateRenderTargetView(renderTargets[n].Get(), nullptr,
            pDevice->GetDescriptorHeapManager()->GetRTVHandle(n));
    }

    // Create the command allocator.
    ThrowIfFailed(pDevice->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));

    // Create the command list.
    pCommandList = new D3D12CommandList(pDevice->GetDevice(), commandAllocator);
}

// Load the sample assets.
void MiniEngine::LoadAssets()
{
    // Create scene objects.
    {
        pSceneManager = make_shared<SceneManager>(pDevice);
        pSceneManager->InitFBXImporter();
        pSceneManager->LoadScene();
        pSceneManager->CreateCamera(width, height);

        pDrawObjectPass = make_shared<DrawObjectsPass>(pDevice, pSceneManager);
        pDrawObjectPass->Setup(pCommandList);
    }

    // Create an empty root signature.
    {
        CD3DX12_DESCRIPTOR_RANGE cbvDescriptorTableRanges[2];
        cbvDescriptorTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);
        cbvDescriptorTableRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0);

        CD3DX12_DESCRIPTOR_RANGE srvDescriptorTableRanges[1];
        srvDescriptorTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

        CD3DX12_DESCRIPTOR_RANGE samplerDescriptorTableRanges[1];
        samplerDescriptorTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);

        CD3DX12_ROOT_PARAMETER rootParameters[4];
        rootParameters[CONSTANT_BUFFER_VIEW_GLOBAL].InitAsDescriptorTable(1, &cbvDescriptorTableRanges[0], D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[CONSTANT_BUFFER_VIEW_PEROBJECT].InitAsDescriptorTable(1, &cbvDescriptorTableRanges[1], D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[SHADER_RESOURCE_VIEW].InitAsDescriptorTable(1, &srvDescriptorTableRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[SAMPLER].InitAsDescriptorTable(1, &samplerDescriptorTableRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        // create a static sampler
        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 1;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            signature.GetAddressOf(),
            error.GetAddressOf()));
        ThrowIfFailed(pDevice->GetDevice()->CreateRootSignature(0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = rootSignature.Get();
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
        ThrowIfFailed(pDevice->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(pDevice->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
        pCommandList->FlushResourceBarriers();

        ExecuteCommandList();
        UpdateFence();
    }
}

void MiniEngine::OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case 'A':
        pSceneManager->GetCamera()->MoveAlongX(1);
        break;
    case 'D':
        pSceneManager->GetCamera()->MoveAlongX(-1);
        break;
    case 'W':
        pSceneManager->GetCamera()->MoveAlongZ(1);
        break;
    case 'S':
        pSceneManager->GetCamera()->MoveAlongZ(-1);
        break;

    case 'Q':
        pSceneManager->GetCamera()->RotateAlongY(1);
        break;
    case 'E':
        pSceneManager->GetCamera()->RotateAlongY(-1);
        break;
    case 'Z':
        pSceneManager->GetCamera()->RotateAlongX(1);
        break;
    case 'X':
        pSceneManager->GetCamera()->RotateAlongX(-1);
        break;

    case 'C':
        pSceneManager->GetCamera()->ResetTransform();
        break;
    }
}

void MiniEngine::OnKeyUp(UINT8 key)
{

}

// Update frame-based values.
void MiniEngine::OnUpdate()
{
    // Update scene objects.
    pSceneManager->UpdateTransforms();
    pSceneManager->UpdateCamera();
}

// Render the scene.
void MiniEngine::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Present the frame.
    ThrowIfFailed(pSwapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void MiniEngine::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(fenceEvent);
}

void MiniEngine::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    pCommandList->SetPipelineState(commandAllocator, pipelineState);
    pCommandList->SetRootSignature(rootSignature);

    // Indicate that the back buffer will be used as a render target.
    pCommandList->AddTransitionResourceBarriers(renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    pCommandList->FlushResourceBarriers();

    pDrawObjectPass->Execute(pCommandList, frameIndex);

    // Indicate that the back buffer will now be used to present.
    pCommandList->AddTransitionResourceBarriers(renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    pCommandList->FlushResourceBarriers();

    ExecuteCommandList();
}

void MiniEngine::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 value = UpdateFence();

    // Wait until the previous frame is finished.
    if (fence->GetCompletedValue() < value)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(value, fenceEvent));
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    frameIndex = pSwapChain->GetCurrentBackBufferIndex();
}

void MiniEngine::ExecuteCommandList()
{
    ThrowIfFailed(pCommandList->GetCommandList()->Close());

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { pCommandList->GetCommandList().Get() };
    pDevice->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

UINT64 MiniEngine::UpdateFence()
{
    const UINT64 value = fenceValue;
    ThrowIfFailed(pDevice->GetCommandQueue()->Signal(fence.Get(), value));
    fenceValue++;

    return value;
}
