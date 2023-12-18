#pragma once
#include "D3D12Texture.h"

class ViewManager
{
private:
    const static UINT kGBufferCount = 4;

    std::shared_ptr<D3D12Device> pDevice;
    ComPtr<IDXGISwapChain3> pSwapChain;
    ComPtr<ID3D12Resource> pBackBuffers[FRAME_COUNT];
    std::map<UINT, D3D12Texture*> pRenderTargetViews;
    std::map<UINT, D3D12Texture*> pDepthStencilViews;
    std::map<UINT, D3D12Texture*> pUnorderedAccessViews;

    // Index of handles.
    UINT colorHandles[2];
    UINT gBufferHandle[kGBufferCount];
    UINT dsvHandle;
    UINT uavColorHandle;
    BOOL useFirstHandle;

    UINT frameIndex;
    UINT globalSRVID;
    UINT rtvID;
    UINT dsvID;
    UINT uavID;
    UINT width;
    UINT height;

    // Helper functions
    D3D12_RESOURCE_STATES GetResourceState(D3D12TextureType type, BOOL isPixelShaderResource);

public:
    ViewManager(std::shared_ptr<D3D12Device>&, UINT inWidth, UINT inHeight);
    ~ViewManager();

    static UINT sFrameCount;

    void UpdateFrameIndex();
    UINT CreateRenderTarget();
    UINT CreateDepthStencilView();
    UINT CreateUnorderedAccessView();
    const UINT GetNextColorHandle();
    const UINT GetRTVSRVHandle(UINT rtvHandle);
    const UINT GetDSVSRVHandle(UINT dsvHandle);
    void ConvertTextureType(
        D3D12CommandList*& pCommandList,
        UINT handleIndex,
        D3D12TextureType type,
        D3D12TextureType targetType,
        BOOL isPixelShaderResource = TRUE);

    inline IDXGISwapChain3* GetSwapChain() const { return pSwapChain.Get(); }
    inline const UINT GetCurrentColorHandle() const { return useFirstHandle == TRUE ? colorHandles[0] : colorHandles[1]; }
    inline const UINT GetGBufferHandle(UINT index) const { return gBufferHandle[index]; }
    inline const UINT GetGBufferCount() const { return kGBufferCount; }
    inline const UINT GetCurrentDSVHandle() const { return dsvHandle; }
    inline const UINT GetUAVColorHandle() const{ return uavColorHandle; }
    inline const UINT GetFrameIndex() const { return frameIndex; }

    inline ID3D12Resource* GetCurrentBackBuffer() const { return pBackBuffers[frameIndex].Get(); }
    inline const D3D12Resource* GetCurrentRTVBuffer(UINT rtvHandle) { return pRenderTargetViews[rtvHandle]->GetTextureBuffer(); }
    inline const D3D12Resource* GetUAVBuffer(UINT uavHandle) { return pUnorderedAccessViews[uavHandle]->GetTextureBuffer(); }
};
