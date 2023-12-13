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
    UINT colorHandles[2];
    UINT gBufferHandle;
    BOOL useFirstHandle;

    const UINT kGBufferSize = 2;

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

    static UINT sFrameCount;

    void UpdateFrameIndex();
    UINT CreateRenderTarget();
    void CreateDXRUAV();
    const UINT GetNextColorHandle();
    UINT GetTheSRVHandle(UINT rtHandle);
    void EmplaceRenderTarget(D3D12CommandList*& pCommandList, UINT handleID, D3D12TextureType type);

    inline IDXGISwapChain3* GetSwapChain() const { return pSwapChain.Get(); }
    inline ID3D12Resource* GetCurrentBackBuffer() const { return pBackBuffers[frameIndex].Get(); }
    inline ID3D12Resource* GetCurrentBuffer(UINT rtHandle) { return pRenderTargets[rtHandle]->GetTextureBuffer()->GetResource(); }
    inline const UINT GetCurrentColorHandle() const { return useFirstHandle == TRUE ? colorHandles[0] : colorHandles[1]; }
    inline const UINT GetGBufferHandle() const { return gBufferHandle; }
    inline const UINT GetGBufferSize() const { return kGBufferSize; }
    inline const UINT GetFrameIndex() const { return frameIndex; }
    inline ID3D12Resource* GetRayTracingOutput() const { return pRayTracingOutput->GetTextureBuffer()->GetResource(); }
};
