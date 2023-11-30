#pragma once
#include "D3D12Texture.h"

class ViewManager
{
private:
    std::shared_ptr<D3D12Device> pDevice;
    ComPtr<IDXGISwapChain3> pSwapChain;
    ComPtr<ID3D12Resource> pBackBuffers[FRAME_COUNT];
    std::map<UINT, D3D12Texture*> pRenderTargets;
    D3D12Texture* pDepthStencil;
    D3D12Texture* pRayTracingOutput;
    UINT colorHandle;

    UINT frameIndex;
    UINT globalSRVID;
    UINT rtvID;
    UINT width;
    UINT height;

    // Helper functions
    D3D12_RESOURCE_STATES GetResourceState(D3D12TextureType type);

public:
    ViewManager(std::shared_ptr<D3D12Device>&, UINT inWidth, UINT inHeight);
    ~ViewManager();

    UINT CreateRenderTarget();
    void CreateDXRUAV();
    void EmplaceRenderTarget(D3D12CommandList*& pCommandList, UINT handleID, D3D12TextureType type);
    UINT GetTheSRVHandle(UINT rtHandle);

    inline void UpdateFrameIndex() { frameIndex = pSwapChain->GetCurrentBackBufferIndex(); }
    inline IDXGISwapChain3* GetSwapChain() const { return pSwapChain.Get(); }
    inline ID3D12Resource* GetCurrentBackBuffer() const { return pBackBuffers[frameIndex].Get(); }
    inline const UINT GetColorHandle() const { return colorHandle; }
    inline const UINT GetFrameIndex() const { return frameIndex; }
    inline ID3D12Resource* GetRayTracingOutput() const { return pRayTracingOutput->GetTextureBuffer()->GetResource(); }
};
