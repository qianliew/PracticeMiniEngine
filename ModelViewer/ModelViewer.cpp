//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "ModelViewer.h"

ModelViewer::ModelViewer(UINT width, UINT height, std::wstring name) :
    DXSample(width, height, name),
    m_frameIndex(0),
    m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
    m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
    m_rtvDescriptorSize(0)
{
}

void ModelViewer::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void ModelViewer::LoadPipeline()
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

    if (m_useWarpDevice)
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }
    else
    {
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), &hardwareAdapter);

        ThrowIfFailed(D3D12CreateDevice(
            hardwareAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&m_device)
            ));
    }

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FrameCount;
    swapChainDesc.Width = m_width;
    swapChainDesc.Height = m_height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(factory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Win32Application::GetHwnd(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));

    ThrowIfFailed(swapChain.As(&m_swapChain));
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a constant buffer view (CBV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors = 1;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_cbvHeap)));

        // Describe and create a shader resource view (SRV) descriptor heap.

        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
            m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

// Load the sample assets.
void ModelViewer::LoadAssets()
{
    // Create scene objects.
    {
        m_camera = std::make_shared<Camera>();
        m_constant = std::make_shared<Constant>();
        m_mesh = std::make_shared<Mesh>();
        m_texture = std::make_shared<Texture>();
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

        D3D12_ROOT_DESCRIPTOR_TABLE cbvDescriptorTable;
        cbvDescriptorTable.NumDescriptorRanges = _countof(cbvDescriptorTableRanges);
        cbvDescriptorTable.pDescriptorRanges = &cbvDescriptorTableRanges[0];

        D3D12_ROOT_DESCRIPTOR_TABLE srvDescriptorTable;
        srvDescriptorTable.NumDescriptorRanges = _countof(srvDescriptorTableRanges);
        srvDescriptorTable.pDescriptorRanges = &srvDescriptorTableRanges[0];

        CD3DX12_ROOT_PARAMETER rootParameters[2];
        rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[0].DescriptorTable = cbvDescriptorTable;
        rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters[1].DescriptorTable = srvDescriptorTable;
        rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // create a static sampler
        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
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
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf()));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
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
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Create the constant buffer.
    {
        m_constantBuffer = std::make_unique<UploadBuffer>();
        m_constantBuffer->CreateConstantBuffer(m_device.Get(), sizeof(Constant));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = m_constantBuffer->GetBuffer()->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = CalculateConstantBufferByteSize(m_constantBuffer->GetDataSize());

        m_device->CreateConstantBufferView(&cbvDesc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Create the vertex and index buffer.
    {
        m_vertexBuffer = std::make_unique<UploadBuffer>();
        m_vertexStaticBuffer = std::make_unique<DefaultBuffer>();
        m_indexBuffer = std::make_unique<UploadBuffer>();
        m_indexStaticBuffer = std::make_unique<DefaultBuffer>();
        const UINT vertexBufferSize = m_mesh->GetVerticesSize();
        const UINT indexBufferSize = m_mesh->GetIndicesSize();

        D3D12_RESOURCE_DESC vertexDesc = {};
        vertexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        vertexDesc.Alignment = 0;
        vertexDesc.Width = vertexBufferSize;
        vertexDesc.Height = 1;
        vertexDesc.DepthOrArraySize = 1;
        vertexDesc.MipLevels = 1;
        vertexDesc.Format = DXGI_FORMAT_UNKNOWN;
        vertexDesc.SampleDesc.Count = 1;
        vertexDesc.SampleDesc.Quality = 0;
        vertexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        vertexDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // Initialize the vertex buffer.
        m_vertexBuffer->CreateBuffer(m_device.Get(), vertexBufferSize);
        m_vertexBuffer->CopyData(m_mesh->GetVerticesData());
        m_vertexStaticBuffer->CreateBuffer(m_device.Get(), &vertexDesc);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexStaticBuffer->GetBuffer()->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;

        D3D12_RESOURCE_DESC indexDesc = {};
        indexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        indexDesc.Alignment = 0;
        indexDesc.Width = indexBufferSize;
        indexDesc.Height = 1;
        indexDesc.DepthOrArraySize = 1;
        indexDesc.MipLevels = 1;
        indexDesc.Format = DXGI_FORMAT_UNKNOWN;
        indexDesc.SampleDesc.Count = 1;
        indexDesc.SampleDesc.Quality = 0;
        indexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        indexDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        // Initialize the index buffer.
        m_indexBuffer->CreateBuffer(m_device.Get(), indexBufferSize);
        m_indexBuffer->CopyData(m_mesh->GetIndicesData());
        m_indexStaticBuffer->CreateBuffer(m_device.Get(), &indexDesc);

        // Initialize the index buffer view.
        m_indexBufferView.BufferLocation = m_indexStaticBuffer->GetBuffer()->GetGPUVirtualAddress();
        m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        m_indexBufferView.SizeInBytes = indexBufferSize;
    }

    // Load textures.
    {
        m_texture->LoadTexture(L"test.png");
        m_textureBuffer = std::make_unique<UploadBuffer>();
        m_textureStaticBuffer = std::make_unique<DefaultBuffer>();

        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Alignment = 0;
        textureDesc.Width = m_texture->GetTextureWidth();
        textureDesc.Height = m_texture->GetTextureHeight();
        textureDesc.DepthOrArraySize = 1;
        textureDesc.MipLevels = 1;
        textureDesc.Format = m_texture->GetTextureDXGIFormat();
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        UINT64 textureBufferSize;
        m_device.Get()->GetCopyableFootprints(&textureDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureBufferSize);
        m_textureBuffer->CreateBuffer(m_device.Get(), textureBufferSize);
        m_textureStaticBuffer->CreateBuffer(m_device.Get(), &textureDesc);

        // Init texture data.
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = m_texture->GetTextureData();
        textureData.RowPitch = m_texture->GetTextureBytesPerRow();
        textureData.SlicePitch = textureData.RowPitch * textureDesc.Height;

        // Update texture data from upload buffer to gpu buffer.
        UpdateSubresources(m_commandList.Get(), m_textureStaticBuffer->GetBuffer().Get(), m_textureBuffer->GetBuffer().Get(), 0, 0, 1, &textureData);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        m_device->CreateShaderResourceView(m_textureStaticBuffer->GetBuffer().Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();

        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_textureStaticBuffer->GetBuffer().Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
        m_commandList->CopyResource(m_vertexStaticBuffer->GetBuffer().Get(), m_vertexBuffer->GetBuffer().Get());
        m_commandList->CopyResource(m_indexStaticBuffer->GetBuffer().Get(), m_indexBuffer->GetBuffer().Get());
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexStaticBuffer->GetBuffer().Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
        m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexStaticBuffer->GetBuffer().Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
        ThrowIfFailed(m_commandList->Close());

        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        const UINT64 fence = m_fenceValue;
        ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
        m_fenceValue++;

        m_texture->ReleaseTexture();
        m_vertexBuffer.release();
        m_indexBuffer.release();
    }
}

void ModelViewer::OnKeyDown(UINT8 key)
{
    switch (key)
    {
        case 'A':
            m_camera->MoveCameraAlongX(1);
            break;
        case 'D':
            m_camera->MoveCameraAlongX(-1);
            break;
        case 'W':
            m_camera->MoveCameraAlongZ(1);
            break;
        case 'S':
            m_camera->MoveCameraAlongZ(-1);
            break;
            
        case 'Q':
            m_camera->RotateCameraAlongY(1);
            break;
        case 'E':
            m_camera->RotateCameraAlongY(-1);
            break;
        case 'Z':
            m_camera->RotateCameraAlongX(1);
            break;
        case 'X':
            m_camera->RotateCameraAlongX(-1);
            break;

        case 'C':
            m_camera->ResetCamera();
            break;
    }
}

void ModelViewer::OnKeyUp(UINT8 key)
{

}

// Update frame-based values.
void ModelViewer::OnUpdate()
{
    // Update scene objects.
    XMStoreFloat4x4(&m_constant->ObjectToWorldMatrix, m_camera->GetMVPMatrix());

    m_constant->a.x = 1;
    m_constant->a.y = 0;
    m_constant->a.z = 1;
    m_constant->a.w = 1;

    m_constantBuffer->CopyData(m_constant.get());
}

// Render the scene.
void ModelViewer::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(m_swapChain->Present(1, 0));

    WaitForPreviousFrame();
}

void ModelViewer::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(m_fenceEvent);
}

void ModelViewer::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    ID3D12DescriptorHeap* descriptorHeaps[] = { m_cbvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    m_commandList->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

    descriptorHeaps[0] = m_srvHeap.Get();
    m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
    m_commandList->SetGraphicsRootDescriptorTable(1, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

    // Set necessary state.
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->IASetIndexBuffer(&m_indexBufferView);

    m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

    // Indicate that the back buffer will now be used to present.
    m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(m_commandList->Close());
}

void ModelViewer::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
