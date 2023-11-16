#pragma once
#include "D3D12Texture.h"

class ViewManager
{
private:
    std::shared_ptr<D3D12Device> pDevice;
    ComPtr<IDXGISwapChain3> pSwapChain;
    ComPtr<ID3D12Resource> pBackBuffers[FRAME_COUNT];
    D3D12Texture* pRenderTarget;
    D3D12Texture* pDepthStencil;
    D3D12Texture* pRayTracingOutput;

    UINT globalSRVID;
    UINT rtvID;
    UINT width;
    UINT height;

    // Helper functions
    D3D12_RESOURCE_STATES GetResourceState(D3D12TextureType type);

public:
    ViewManager(std::shared_ptr<D3D12Device>&, UINT inWidth, UINT inHeight);
    ~ViewManager();

    void EmplaceRenderTarget(D3D12CommandList*& pCommandList, D3D12TextureType type);

    inline IDXGISwapChain3* GetSwapChain() const { return pSwapChain.Get(); }
    inline ID3D12Resource* GetBackBufferAt(UINT index) const { return pBackBuffers[index].Get(); }
    inline ID3D12Resource* GetRayTracingOutput() const { return pRayTracingOutput->GetTextureBuffer()->GetResource(); }
};
