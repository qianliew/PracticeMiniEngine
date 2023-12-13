#include "stdafx.h"
#include "ViewManager.h"
#include "Window.h"

ViewManager::ViewManager(std::shared_ptr<D3D12Device>& device, UINT inWidth, UINT inHeight) :
    pDevice(device),
    width(inWidth),
    height(inHeight),
    globalSRVID(0),
    rtvID(FRAME_COUNT)
{
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

    // Setup the frame index.
    frameIndex = pSwapChain->GetCurrentBackBufferIndex();

    // Create frame resources.
    // Create a RTV for each frame.
    for (UINT n = 0; n < FRAME_COUNT; n++)
    {
        ThrowIfFailed(pSwapChain->GetBuffer(n, IID_PPV_ARGS(&pBackBuffers[n])));
        pDevice->GetDevice()->CreateRenderTargetView(pBackBuffers[n].Get(), nullptr,
            pDevice->GetDescriptorHeapManager()->GetHandle(RENDER_TARGET_VIEW, n));
    }

    // Create a render target for the color buffer.
    colorHandles[0] = CreateRenderTarget();
    colorHandles[1] = CreateRenderTarget();
    useFirstHandle = TRUE;

    // Create render targets for the GBuffer.
    gBufferHandle = CreateRenderTarget();
    CreateRenderTarget();

    // Create a depth stencil buffer.
    pDepthStencil = new D3D12Texture(-1, -1, width, height);
    pDepthStencil->CreateTexture(D3D12TextureType::DepthStencil);

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pDepthStencil->GetTextureBuffer(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue);
    pDepthStencil->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(DEPTH_STENCIL_VIEW, 0));
}

ViewManager::~ViewManager()
{
    for (auto it = pRenderTargets.begin(); it != pRenderTargets.end(); it++)
    {
        delete it->second;
    }
    delete pDepthStencil;
}

UINT ViewManager::sFrameCount = 0;

void ViewManager::UpdateFrameIndex()
{
    frameIndex = pSwapChain->GetCurrentBackBufferIndex();
    sFrameCount++;
}

UINT ViewManager::CreateRenderTarget()
{
    D3D12Texture* pRenderTarget = new D3D12Texture(globalSRVID++, rtvID++, width, height);
    pRenderTarget->CreateTexture(D3D12TextureType::RenderTarget);

    D3D12_CLEAR_VALUE renderTargetClearValue = {};
    renderTargetClearValue.Color[0] = 0.0f;
    renderTargetClearValue.Color[1] = 0.0f;
    renderTargetClearValue.Color[2] = 0.0f;
    renderTargetClearValue.Color[3] = 1.0f;
    renderTargetClearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pRenderTarget->GetTextureBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        &renderTargetClearValue);

    const UINT rtid = pRenderTarget->GetRenderTargetID();
    pRenderTarget->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(RENDER_TARGET_VIEW, rtid));

    pRenderTargets[rtid] = pRenderTarget;

    return rtid;
}

void ViewManager::CreateDXRUAV()
{
    // Create 2D output texture for raytracing.
    pRayTracingOutput = new D3D12Texture(-1, -1, width, height);
    pRayTracingOutput->CreateTexture(D3D12TextureType::UnorderedAccess);

    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pRayTracingOutput->GetTextureBuffer(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr);
    pRayTracingOutput->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(UNORDERED_ACCESS_VIEW, 0));
}

UINT ViewManager::GetTheSRVHandle(UINT rtHandle)
{
    if (pRenderTargets[rtHandle] != nullptr)
    {
        return pRenderTargets[rtHandle]->GetTextureID();
    }
    return 0;
}

const UINT ViewManager::GetNextColorHandle()
{
    useFirstHandle = !useFirstHandle;
    return GetCurrentColorHandle();
}

void ViewManager::EmplaceRenderTarget(D3D12CommandList*& pCommandList, UINT handleID, D3D12TextureType type)
{
    D3D12_RESOURCE_STATES stateBefore = GetResourceState(pRenderTargets[handleID]->GetType());
    UINT heapMapIndex = type == D3D12TextureType::ShaderResource ? SHADER_RESOURCE_VIEW_GLOBAL
        : RENDER_TARGET_VIEW;
    UINT offset = type == D3D12TextureType::ShaderResource ? pRenderTargets[handleID]->GetTextureID() : handleID;

    pRenderTargets[handleID]->CreateTexture(type);
    pRenderTargets[handleID]->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(heapMapIndex, offset));
    pCommandList->AddTransitionResourceBarriers(
        pRenderTargets[handleID]->GetTextureBuffer()->GetResource(),
        stateBefore,
        GetResourceState(type));
    pCommandList->FlushResourceBarriers();
}

// Helper functions
D3D12_RESOURCE_STATES ViewManager::GetResourceState(D3D12TextureType type)
{
    switch (type)
    {
    case D3D12TextureType::RenderTarget:
        return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case D3D12TextureType::ShaderResource:
        return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }
}
