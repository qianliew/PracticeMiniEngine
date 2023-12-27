#include "stdafx.h"
#include "ViewManager.h"
#include "Window.h"

ViewManager::ViewManager(std::shared_ptr<D3D12Device>& device, UINT inWidth, UINT inHeight) :
    pDevice(device),
    width(inWidth),
    height(inHeight),
    globalSRVID(0),
    rtvID(FRAME_COUNT),
    dsvID(0),
    uavID(0)
{
    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
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
    for (UINT i = 0; i < GetGBufferCount(); i++)
    {
        gBufferHandle[i] = CreateRenderTarget();
    }

    dsvHandle = CreateDepthStencilView();
    uavColorHandle = CreateUnorderedAccessView();
}

ViewManager::~ViewManager()
{
    for (auto it = pRenderTargetViews.begin(); it != pRenderTargetViews.end(); it++)
    {
        delete it->second;
    }
    for (auto it = pDepthStencilViews.begin(); it != pDepthStencilViews.end(); it++)
    {
        delete it->second;
    }
}

UINT ViewManager::sFrameCount = 0;

void ViewManager::UpdateFrameIndex()
{
    frameIndex = pSwapChain->GetCurrentBackBufferIndex();
    sFrameCount++;
}

UINT ViewManager::CreateRenderTarget()
{
    D3D12Texture* pRenderTarget = new D3D12Texture(globalSRVID++, rtvID++, width, height,
        D3D12TextureType::RenderTarget, DXGI_FORMAT_R16G16B16A16_FLOAT);
    pRenderTarget->CreateTextureResource();

    D3D12_CLEAR_VALUE renderTargetClearValue = {};
    renderTargetClearValue.Color[0] = 0.0f;
    renderTargetClearValue.Color[1] = 0.0f;
    renderTargetClearValue.Color[2] = 0.0f;
    renderTargetClearValue.Color[3] = 1.0f;
    renderTargetClearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pRenderTarget->GetTextureBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        L"RenderTargetView",
        &renderTargetClearValue);

    const UINT rtvHandle = pRenderTarget->GetRTVHandle();
    pRenderTarget->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(RENDER_TARGET_VIEW, rtvHandle));

    pRenderTargetViews[rtvHandle] = pRenderTarget;

    return rtvHandle;
}

// Create a depth stencil buffer.
UINT ViewManager::CreateDepthStencilView()
{
    D3D12Texture* pDepthStencil = new D3D12Texture(globalSRVID++, dsvID++, width, height,
        D3D12TextureType::DepthStencil, DXGI_FORMAT_R32_TYPELESS);
    pDepthStencil->CreateTextureResource();

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pDepthStencil->GetTextureBuffer(),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        L"DepthStencilView",
        &depthOptimizedClearValue);

    const UINT dsvHandle = pDepthStencil->GetDSVHandle();
    pDepthStencil->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(DEPTH_STENCIL_VIEW, dsvHandle));

    pDepthStencilViews[dsvHandle] = pDepthStencil;
    return dsvHandle;
}

UINT ViewManager::CreateUnorderedAccessView()
{
    D3D12Texture* pUAV = new D3D12Texture(globalSRVID++, rtvID++, width, height,
        D3D12TextureType::UnorderedAccess, DXGI_FORMAT_R16G16B16A16_FLOAT);
    pUAV->CreateTextureResource();

    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pUAV->GetTextureBuffer(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        nullptr);

    const UINT uavHandle = pUAV->GetUAVHandle();
    pUAV->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(UNORDERED_ACCESS_VIEW, 0));

    pUnorderedAccessViews[uavHandle] = pUAV;
    return uavHandle;
}

const UINT ViewManager::GetRTVSRVHandle(UINT rtvHandle)
{
    if (pRenderTargetViews[rtvHandle] != nullptr
        && pRenderTargetViews[rtvHandle]->GetType() == D3D12TextureType::ShaderResource)
    {
        return pRenderTargetViews[rtvHandle]->GetTextureID();
    }
    return 0;
}

const UINT ViewManager::GetDSVSRVHandle(UINT dsvHandle)
{
    if (pDepthStencilViews[dsvHandle] != nullptr
        && pDepthStencilViews[dsvHandle]->GetType() == D3D12TextureType::ShaderResource)
    {
        return pDepthStencilViews[dsvHandle]->GetTextureID();
    }
    return 0;
}

const UINT ViewManager::GetNextColorHandle()
{
    useFirstHandle = !useFirstHandle;
    return GetCurrentColorHandle();
}

void ViewManager::ConvertTextureType(
    D3D12CommandList*& pCommandList,
    UINT handleIndex,
    D3D12TextureType type,
    D3D12TextureType targetType,
    BOOL isPixelShaderResource)
{
    UINT heapMapIndex = targetType == D3D12TextureType::ShaderResource ? SHADER_RESOURCE_VIEW_GLOBAL
        : targetType == D3D12TextureType::DepthStencil ? DEPTH_STENCIL_VIEW
        : RENDER_TARGET_VIEW;

    auto convert = [&](D3D12Texture*& resource)
    {
        D3D12_RESOURCE_STATES stateBefore = GetResourceState(resource->GetType(), isPixelShaderResource);
        UINT offset = targetType == D3D12TextureType::ShaderResource ? resource->GetTextureID()
            : targetType == D3D12TextureType::DepthStencil ? 0
            : handleIndex;

        resource->ChangeTextureType(targetType);
        resource->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
            pDevice->GetDescriptorHeapManager()->GetHandle(heapMapIndex, offset));
        pCommandList->AddTransitionResourceBarriers(
            resource->GetTextureBuffer()->GetResource().Get(),
            stateBefore,
            GetResourceState(targetType, isPixelShaderResource));
    };

    convert(type == D3D12TextureType::DepthStencil ? pDepthStencilViews[0] : pRenderTargetViews[handleIndex]);
    pCommandList->FlushResourceBarriers();
}

// Helper functions
D3D12_RESOURCE_STATES ViewManager::GetResourceState(D3D12TextureType type, BOOL isPixelShaderResource)
{
    switch (type)
    {
    case D3D12TextureType::ShaderResource:
        return isPixelShaderResource ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
    case D3D12TextureType::DepthStencil:
        return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case D3D12TextureType::RenderTarget:
        return D3D12_RESOURCE_STATE_RENDER_TARGET;
    }
}
