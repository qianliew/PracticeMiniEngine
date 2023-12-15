#pragma once
#include "D3D12Texture.h"

class ViewManager
{
private:
    const static UINT kGBufferCount = 3;

    std::shared_ptr<D3D12Device> pDevice;
    ComPtr<IDXGISwapChain3> pSwapChain;
    ComPtr<ID3D12Resource> pBackBuffers[FRAME_COUNT];
    std::map<UINT, D3D12Texture*> pRenderTargets;
    D3D12Texture* pDepthStencil;
    D3D12Texture* pRayTracingOutput;
    UINT colorHandles[2];
    UINT gBufferHandle[kGBufferCount];
    BOOL useFirstHandle;

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
    UINT GetSRVHandle4RTV(UINT rtvHandle);
    void ConvertTextureType(D3D12CommandList*& pCommandList, UINT handleIndex, D3D12TextureType type, D3D12TextureType targetType);

    inline IDXGISwapChain3* GetSwapChain() const { return pSwapChain.Get(); }
    inline ID3D12Resource* GetCurrentBackBuffer() const { return pBackBuffers[frameIndex].Get(); }
    inline const D3D12Resource* GetCurrentBuffer(UINT rtHandle) { return pRenderTargets[rtHandle]->GetTextureBuffer(); }
    inline const UINT GetCurrentColorHandle() const { return useFirstHandle == TRUE ? colorHandles[0] : colorHandles[1]; }
    inline const UINT GetDepthSRVHandle() const { return pDepthStencil->GetTextureID();  }
    inline const UINT GetGBufferHandle(UINT index) const { return gBufferHandle[index]; }
    inline const UINT GetGBufferCount() const { return kGBufferCount; }
    inline const UINT GetFrameIndex() const { return frameIndex; }
    inline ID3D12Resource* GetDepthStencil() const { return pDepthStencil->GetTextureBuffer()->GetResource().Get(); }
    inline ID3D12Resource* GetRayTracingOutput() const { return pRayTracingOutput->GetTextureBuffer()->GetResource().Get(); }
};
