#pragma once

#include "Win32Application.h"
#include "FBXImporter.h"
#include "D3D12Camera.h"
#include "D3D12Model.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::shared_ptr;

class MiniEngine
{
public:
    MiniEngine(UINT width, UINT height, std::wstring name);
    ~MiniEngine();

    void OnInit();
    void OnUpdate();
    void OnRender();
    void OnDestroy();
    void OnKeyDown(UINT8 /*key*/);
    void OnKeyUp(UINT8 /*key*/);

    // Accessors.
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    const WCHAR* GetTitle() const { return m_title.c_str(); }

    void ParseCommandLineArgs(_In_reads_(argc) WCHAR* argv[], int argc);

protected:
    std::wstring GetAssetFullPath(LPCWSTR assetName);

    void GetHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);

    void SetCustomWindowText(LPCWSTR text);

    // Viewport dimensions.
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;

    // Adapter info.
    bool m_useWarpDevice;

private:
    // Root assets path.
    std::wstring m_assetsPath;

    // Window title.
    std::wstring m_title;

    static const UINT FrameCount = 2;

    struct Constant
    {
        XMFLOAT4X4 ObjectToWorldMatrix;
        XMFLOAT4 a;
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource> m_depthStencils[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
    UINT m_dsvDescriptorSize;

    unique_ptr<D3D12BufferManager> m_allocator;
    unique_ptr<D3D12DescriptorHeapManager> m_descriptorHeapManager;

    // App resources.
    unique_ptr<D3D12UploadBuffer> m_constantBuffer;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    // Scene objects
    shared_ptr<Constant> m_constant;
    shared_ptr<D3D12Camera> m_camera;
    shared_ptr<D3D12Model> m_model;
    unique_ptr<FBXImporter> m_fbxImporter;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};