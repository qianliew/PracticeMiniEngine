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

        // Describe and create a constant buffer view (CBV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors = 1;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap)));
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

    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
}

// Load the sample assets.
void MiniEngine::LoadAssets()
{
    // Create scene objects.
    {
        allocator = std::make_unique<D3D12BufferManager>(device);
        camera = std::make_shared<D3D12Camera>(static_cast<FLOAT>(width), static_cast<FLOAT>(height));
        camera->SetViewport(static_cast<FLOAT>(width), static_cast<FLOAT>(height));
        camera->SetScissorRect(static_cast<LONG>(width), static_cast<LONG>(height));

        fbxImporter = std::make_unique<FBXImporter>();
        fbxImporter->InitializeSdkObjects();
        model = std::make_shared<D3D12Model>((char*)"cube.fbx", (char*)"test.png");
        model->LoadModel(fbxImporter);
    }

    // Create an empty root signature.
    {
        D3D12_DESCRIPTOR_RANGE cbvDescriptorTableRanges[1];
        cbvDescriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        cbvDescriptorTableRanges[0].NumDescriptors = 1;
        cbvDescriptorTableRanges[0].BaseShaderRegister = 0;
        cbvDescriptorTableRanges[0].RegisterSpace = 0;
        cbvDescriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_DESCRIPTOR_RANGE srvDescriptorTableRanges[1];
        srvDescriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvDescriptorTableRanges[0].NumDescriptors = 1;
        srvDescriptorTableRanges[0].BaseShaderRegister = 0;
        srvDescriptorTableRanges[0].RegisterSpace = 0;
        srvDescriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_DESCRIPTOR_RANGE samplerDescriptorTableRanges[1];
        samplerDescriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        samplerDescriptorTableRanges[0].NumDescriptors = 1;
        samplerDescriptorTableRanges[0].BaseShaderRegister = 0;
        samplerDescriptorTableRanges[0].RegisterSpace = 0;
        samplerDescriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_DESCRIPTOR_TABLE cbvDescriptorTable;
        cbvDescriptorTable.NumDescriptorRanges = _countof(cbvDescriptorTableRanges);
        cbvDescriptorTable.pDescriptorRanges = &cbvDescriptorTableRanges[0];

        D3D12_ROOT_DESCRIPTOR_TABLE srvDescriptorTable;
        srvDescriptorTable.NumDescriptorRanges = _countof(srvDescriptorTableRanges);
        srvDescriptorTable.pDescriptorRanges = &srvDescriptorTableRanges[0];

        D3D12_ROOT_DESCRIPTOR_TABLE samplerDescriptorTable;
        samplerDescriptorTable.NumDescriptorRanges = _countof(samplerDescriptorTableRanges);
        samplerDescriptorTable.pDescriptorRanges = &samplerDescriptorTableRanges[0];

        CD3DX12_ROOT_PARAMETER rootParameters[3];
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[0].DescriptorTable = cbvDescriptorTable;
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[1].DescriptorTable = srvDescriptorTable;
        rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[2].DescriptorTable = samplerDescriptorTable;
        rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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

    // Create the command list.
    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)));

    // Create the constant buffer.
    {
        model->GetConstant()->CreateConstantBuffer(device.Get(), sizeof(Matrices));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = model->GetConstant()->ResourceLocation->Resource->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = CalculateConstantBufferByteSize(model->GetConstant()->GetDataSize());

        device->CreateConstantBufferView(&cbvDesc, cbvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Create the vertex and index buffer.
    {
        D3D12UploadBuffer* tempVertexBuffer = new D3D12UploadBuffer();
        allocator->AllocateUploadBuffer(tempVertexBuffer, UploadBufferType::Vertex);
        allocator->AllocateDefaultBuffer(model->GetMesh()->VertexBuffer.get());
        tempVertexBuffer->CopyData(model->GetMesh()->GetVerticesData(), model->GetMesh()->GetVerticesSize());

        D3D12UploadBuffer* tempIndexBuffer = new D3D12UploadBuffer();
        allocator->AllocateUploadBuffer(tempIndexBuffer, UploadBufferType::Index);
        allocator->AllocateDefaultBuffer(model->GetMesh()->IndexBuffer.get());
        tempIndexBuffer->CopyData(model->GetMesh()->GetIndicesData(), model->GetMesh()->GetIndicesSize());

        model->GetMesh()->CreateView();
        commandList->CopyBufferRegion(model->GetMesh()->VertexBuffer->GetResource(),
            0,
            tempVertexBuffer->ResourceLocation->Resource.Get(),
            0,
            model->GetMesh()->GetVerticesSize());
        commandList->CopyBufferRegion(model->GetMesh()->IndexBuffer->GetResource(),
            0,
            tempIndexBuffer->ResourceLocation->Resource.Get(),
            0,
            model->GetMesh()->GetIndicesSize());
    }

    // Load textures.
    {
        D3D12UploadBuffer* tempBuffer = new D3D12UploadBuffer();

        allocator->AllocateUploadBuffer(tempBuffer, UploadBufferType::Texture);
        allocator->AllocateDefaultBuffer(model->GetTexture()->TextureBuffer.get());

        // Init texture data.
        device.Get()->GetCopyableFootprints(model->GetTexture()->TextureBuffer->GetResourceDesc(), 0, 1, 0, nullptr,
            model->GetTexture()->GetTextureHeight(), model->GetTexture()->GetTextureBytesPerRow(), nullptr);
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = model->GetTexture()->GetTextureData();
        textureData.RowPitch = *model->GetTexture()->GetTextureBytesPerRow();
        textureData.SlicePitch = *model->GetTexture()->GetTextureBytesPerRow() * *model->GetTexture()->GetTextureHeight();

        // Update texture data from upload buffer to gpu buffer.
        UpdateSubresources(commandList.Get(), model->GetTexture()->TextureBuffer->GetResource(),
            tempBuffer->ResourceLocation->Resource.Get(), 0, 0, 1, &textureData);

        model->GetTexture()->TextureBuffer->CreateView();
        descriptorHeapManager->GetSRVHandle(model->GetTexture()->TextureBuffer->View, 0);
        device->CreateShaderResourceView(model->GetTexture()->TextureBuffer->GetResource(),
            &model->GetTexture()->TextureBuffer->View->SRVDesc,
            model->GetTexture()->TextureBuffer->View->CPUHandle);

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

        commandList->ResourceBarrier(1,
            &CD3DX12_RESOURCE_BARRIER::Transition(model->GetTexture()->TextureBuffer->GetResource(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        commandList->ResourceBarrier(1,
            &CD3DX12_RESOURCE_BARRIER::Transition(model->GetMesh()->VertexBuffer->GetResource(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
        commandList->ResourceBarrier(1,
            &CD3DX12_RESOURCE_BARRIER::Transition(model->GetMesh()->IndexBuffer->GetResource(),
                D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
        ThrowIfFailed(commandList->Close());

        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        const UINT64 value = fenceValue;
        ThrowIfFailed(commandQueue->Signal(fence.Get(), value));
        fenceValue++;

        model->GetTexture()->ReleaseTexture();
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
    Matrices matrices = model->GetMatrices();
    model->SetObjectToWorldMatrix();
    XMStoreFloat4x4(&matrices.WorldToProjectionMatrix, camera->GetVPMatrix());

    model->GetConstant()->CopyData(&matrices);
}

// Render the scene.
void MiniEngine::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(swapChain->Present(1, 0));

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
    ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState.Get()));

    commandList->SetGraphicsRootSignature(rootSignature.Get());
    ID3D12DescriptorHeap* descriptorHeaps[] = { cbvHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    commandList->SetGraphicsRootDescriptorTable(0, cbvHeap->GetGPUDescriptorHandleForHeapStart());

    descriptorHeapManager->SetSRVs(commandList);
    descriptorHeapManager->SetSamplers(commandList);

    // Set camera relating state.
    commandList->RSSetViewports(1, camera->GetViewport());
    commandList->RSSetScissorRects(1, camera->GetScissorRect());

    // Indicate that the back buffer will be used as a render target.
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencils[frameIndex].Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, dsvDescriptorSize);
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &model->GetMesh()->VertexBuffer->View->VertexBufferView);
    commandList->IASetIndexBuffer(&model->GetMesh()->IndexBuffer->View->IndexBufferView);

    commandList->DrawIndexedInstanced(model->GetMesh()->GetIndicesNum(), 1, 0, 0, 0);

    // Indicate that the back buffer will now be used to present.
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencils[frameIndex].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COMMON));

    ThrowIfFailed(commandList->Close());
}

void MiniEngine::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 value = fenceValue;
    ThrowIfFailed(commandQueue->Signal(fence.Get(), value));
    fenceValue++;

    // Wait until the previous frame is finished.
    if (fence->GetCompletedValue() < value)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(value, fenceEvent));
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    frameIndex = swapChain->GetCurrentBackBufferIndex();
}
