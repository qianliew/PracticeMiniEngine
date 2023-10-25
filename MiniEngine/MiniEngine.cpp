#include "stdafx.h"
#include "MiniEngine.h"

using namespace Microsoft::WRL;

MiniEngine::MiniEngine(UINT width, UINT height, std::wstring name) :
    Window(width, height, name),
    frameIndex(0),
    rtvDescriptorSize(0),
    useWarpDevice(false)
{
    WCHAR path[512];
    GetAssetsPath(path, _countof(path));
    assetsPath = path;
}

MiniEngine::~MiniEngine()
{
}

// Helper function for resolving the full path of assets.
std::wstring MiniEngine::GetAssetFullPath(LPCWSTR assetName)
{
    return assetsPath + assetName;
}

// Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
// If no such adapter can be found, *ppAdapter will be set to nullptr.
_Use_decl_annotations_
void MiniEngine::GetHardwareAdapter(
    IDXGIFactory1* pFactory,
    IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if (adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    *ppAdapter = adapter.Detach();
}

void MiniEngine::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void MiniEngine::LoadPipeline()
{
    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

    if (useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&device)
        ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&device)
        ));
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain1;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1
    ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain1.As(&swapChain));
    frameIndex = swapChain->GetCurrentBackBufferIndex();

    descriptorHeapManager = std::make_unique<D3D12DescriptorHeapManager>(device);

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

        rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a depth stencil view (DSV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = FrameCount;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));

        dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
            device->CreateRenderTargetView(renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, rtvDescriptorSize);
        }

        D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
        depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

        D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
        depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
        depthOptimizedClearValue.DepthStencil.Stencil = 0;

        CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a SDV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
                D3D12_RESOURCE_STATE_COMMON,
                &depthOptimizedClearValue,
                IID_PPV_ARGS(&depthStencils[n])
            );

            device->CreateDepthStencilView(depthStencils[n].Get(), &depthStencilDesc, dsvHandle);
            dsvHandle.Offset(1, dsvDescriptorSize);
        }
    }

    // Create the command allocator.
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));

    // Create the command list.
    cmdList = make_unique<D3D12CommandList>(device, commandAllocator);
}

// Load the sample assets.
void MiniEngine::LoadAssets()
{
    // Create scene objects.
    {
        bufferManager = std::make_unique<D3D12BufferManager>(device);
        camera = std::make_shared<D3D12Camera>(0, static_cast<FLOAT>(width), static_cast<FLOAT>(height));
        camera->SetViewport(static_cast<FLOAT>(width), static_cast<FLOAT>(height));
        camera->SetScissorRect(static_cast<LONG>(width), static_cast<LONG>(height));

        fbxImporter = std::make_unique<FBXImporter>();
        fbxImporter->InitializeSdkObjects();

        model = std::make_shared<D3D12Model>(id++, (char*)"cube.fbx", (char*)"test.png");
        model->LoadModel(fbxImporter);
        model->MoveAlongX(10.0f);
        model->MoveAlongZ(5.0f);

        model2 = std::make_shared<D3D12Model>(id++, (char*)"cube.fbx", (char*)"test.png");
        model2->LoadModel(fbxImporter);
        model2->MoveAlongY(2.0f);

        models.push_back(model);
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
        ThrowIfFailed(device->CreateRootSignature(0,
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
        ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
    }

    // Create the constant buffer.
    {
        bufferManager->AllocateGlobalConstantBuffer();
        descriptorHeapManager->GetCBVHandle(bufferManager->GetGlobalConstantBuffer()->GetView(),
            CONSTANT_BUFFER_VIEW_GLOBAL, 0);
        bufferManager->GetGlobalConstantBuffer()->GetView()->CreateView(device);

        UINT id = model->GetObjectID();
        bufferManager->AllocatePerObjectConstantBuffers(id);
        descriptorHeapManager->GetCBVHandle(bufferManager->GetPerObjectConstantBufferAtIndex(id)->GetView(),
            CONSTANT_BUFFER_VIEW_PEROBJECT, id);
        bufferManager->GetPerObjectConstantBufferAtIndex(id)->GetView()->CreateView(device);

        id = model2->GetObjectID();
        bufferManager->AllocatePerObjectConstantBuffers(id);
        descriptorHeapManager->GetCBVHandle(bufferManager->GetPerObjectConstantBufferAtIndex(id)->GetView(),
            CONSTANT_BUFFER_VIEW_PEROBJECT, id);
        bufferManager->GetPerObjectConstantBufferAtIndex(id)->GetView()->CreateView(device);
    }

    // Create the vertex and index buffer.
    {
        D3D12UploadBuffer* tempVertexBuffer = new D3D12UploadBuffer();
        bufferManager->AllocateUploadBuffer(tempVertexBuffer, UploadBufferType::Vertex);
        bufferManager->AllocateDefaultBuffer(model->GetMesh()->VertexBuffer.get());
        tempVertexBuffer->CopyData(model->GetMesh()->GetVerticesData(), model->GetMesh()->GetVerticesSize());

        D3D12UploadBuffer* tempIndexBuffer = new D3D12UploadBuffer();
        bufferManager->AllocateUploadBuffer(tempIndexBuffer, UploadBufferType::Index);
        bufferManager->AllocateDefaultBuffer(model->GetMesh()->IndexBuffer.get());
        tempIndexBuffer->CopyData(model->GetMesh()->GetIndicesData(), model->GetMesh()->GetIndicesSize());

        model->GetMesh()->CreateViewDesc();
        cmdList->CopyBufferRegion(model->GetMesh()->VertexBuffer->GetResource(),
            tempVertexBuffer->ResourceLocation->Resource.Get(),
            model->GetMesh()->GetVerticesSize());
        cmdList->CopyBufferRegion(model->GetMesh()->IndexBuffer->GetResource(),
            tempIndexBuffer->ResourceLocation->Resource.Get(),
            model->GetMesh()->GetIndicesSize());
    }

    // Load textures.
    {
        D3D12UploadBuffer* tempBuffer = new D3D12UploadBuffer();

        bufferManager->AllocateUploadBuffer(tempBuffer, UploadBufferType::Texture);
        bufferManager->AllocateDefaultBuffer(model->GetTexture()->TextureBuffer.get());

        // Init texture data.
        device.Get()->GetCopyableFootprints(model->GetTexture()->TextureBuffer->GetResourceDesc(), 0, 1, 0, nullptr,
            model->GetTexture()->GetTextureHeight(), model->GetTexture()->GetTextureBytesPerRow(), nullptr);
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = model->GetTexture()->GetTextureData();
        textureData.RowPitch = *model->GetTexture()->GetTextureBytesPerRow();
        textureData.SlicePitch = *model->GetTexture()->GetTextureBytesPerRow() * *model->GetTexture()->GetTextureHeight();

        // Update texture data from upload buffer to gpu buffer.
        cmdList->CopyTextureBuffer(model->GetTexture()->TextureBuffer->GetResource(),
            tempBuffer->ResourceLocation->Resource.Get(), 0, 0, 1, &textureData);

        model->GetTexture()->TextureBuffer->View->SetResource(model->GetTexture()->TextureBuffer->GetResource());
        model->GetTexture()->TextureBuffer->CreateViewDesc();
        descriptorHeapManager->GetSRVHandle(model->GetTexture()->TextureBuffer->View, 0);
        model->GetTexture()->TextureBuffer->View->CreateView(device);

        model->GetTexture()->CreateSampler();
        descriptorHeapManager->GetSamplerHandle(model->GetTexture()->TextureSampler.get(), 0);
        device->CreateSampler(&model->GetTexture()->TextureSampler->SamplerDesc,
            model->GetTexture()->TextureSampler->CPUHandle);
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
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

        cmdList->AddTransitionResourceBarriers(model->GetTexture()->TextureBuffer->GetResource(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        cmdList->AddTransitionResourceBarriers(model->GetMesh()->VertexBuffer->GetResource(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        cmdList->AddTransitionResourceBarriers(model->GetMesh()->IndexBuffer->GetResource(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
        cmdList->FlushResourceBarriers();

        ExecuteCommandList();
        UpdateFence();
    }
}

void MiniEngine::OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case 'A':
        camera->MoveAlongX(1);
        break;
    case 'D':
        camera->MoveAlongX(-1);
        break;
    case 'W':
        camera->MoveAlongZ(1);
        break;
    case 'S':
        camera->MoveAlongZ(-1);
        break;

    case 'Q':
        camera->RotateAlongY(1);
        break;
    case 'E':
        camera->RotateAlongY(-1);
        break;
    case 'Z':
        camera->RotateAlongX(1);
        break;
    case 'X':
        camera->RotateAlongX(-1);
        break;

    case 'C':
        camera->ResetTransform();
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
    CameraConstant cameraConstant = camera->GetCameraConstant();
    XMStoreFloat4x4(&cameraConstant.WorldToProjectionMatrix, camera->GetVPMatrix());
    bufferManager->GetGlobalConstantBuffer()->CopyData(&cameraConstant, sizeof(CameraConstant));

    model->SetObjectToWorldMatrix();
    TransformConstant transformConstant = model->GetTransformConstant();
    bufferManager->GetPerObjectConstantBufferAtIndex(model->GetObjectID())->CopyData(&transformConstant, sizeof(TransformConstant));

    model2->SetObjectToWorldMatrix();
    TransformConstant transformConstant2 = model2->GetTransformConstant();
    bufferManager->GetPerObjectConstantBufferAtIndex(model2->GetObjectID())->CopyData(&transformConstant2, sizeof(TransformConstant));
}

// Render the scene.
void MiniEngine::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Present the frame.
    ThrowIfFailed(swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void MiniEngine::OnDestroy()
{
    model->GetTexture()->ReleaseTexture();

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
    cmdList->SetPipelineState(commandAllocator, pipelineState);
    cmdList->SetRootSignature(rootSignature);

    descriptorHeapManager->SetCBVs(cmdList->GetCommandList(), CONSTANT_BUFFER_VIEW_GLOBAL, 0);
    descriptorHeapManager->SetSRVs(cmdList->GetCommandList());
    descriptorHeapManager->SetSamplers(cmdList->GetCommandList());

    // Set camera relating state.
    cmdList->SetViewports(camera->GetViewport());
    cmdList->SetScissorRects(camera->GetScissorRect());

    // Indicate that the back buffer will be used as a render target.
    cmdList->AddTransitionResourceBarriers(renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->AddTransitionResourceBarriers(depthStencils[frameIndex].Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    cmdList->FlushResourceBarriers();

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, dsvDescriptorSize);
    cmdList->SetRenderTargets(1, &rtvHandle, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    cmdList->ClearColor(rtvHandle, clearColor);
    cmdList->ClearDepth(dsvHandle);
    cmdList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    {
        descriptorHeapManager->SetCBVs(cmdList->GetCommandList(), CONSTANT_BUFFER_VIEW_PEROBJECT, 0);

        cmdList->SetVertexBuffers(0, 1, &model->GetMesh()->VertexBuffer->VertexBufferView);
        cmdList->SetIndexBuffer(&model->GetMesh()->IndexBuffer->IndexBufferView);

        cmdList->DrawIndexedInstanced(model->GetMesh()->GetIndicesNum());
    }

    {
        descriptorHeapManager->SetCBVs(cmdList->GetCommandList(), CONSTANT_BUFFER_VIEW_PEROBJECT, 1);
        //commandList->IASetVertexBuffers(0, 1, &model2->GetMesh()->VertexBuffer->View->VertexBufferView);
        //commandList->IASetIndexBuffer(&model2->GetMesh()->IndexBuffer->View->IndexBufferView);

        //commandList->DrawIndexedInstanced(model2->GetMesh()->GetIndicesNum(), 1, 0, 0, 0);
        cmdList->DrawIndexedInstanced(model->GetMesh()->GetIndicesNum());
    }

    // Indicate that the back buffer will now be used to present.
    cmdList->AddTransitionResourceBarriers(renderTargets[frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->AddTransitionResourceBarriers(depthStencils[frameIndex].Get(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON);
    cmdList->FlushResourceBarriers();

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

    frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void MiniEngine::ExecuteCommandList()
{
    ThrowIfFailed(cmdList->GetCommandList()->Close());

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { cmdList->GetCommandList().Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

UINT64 MiniEngine::UpdateFence()
{
    const UINT64 value = fenceValue;
    ThrowIfFailed(commandQueue->Signal(fence.Get(), value));
    fenceValue++;

    return value;
}
