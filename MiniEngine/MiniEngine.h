#pragma once

#include "Window.h"
#include "FBXImporter.h"
#include "D3D12Camera.h"
#include "D3D12Model.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::shared_ptr;

class MiniEngine : public Window
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

    std::wstring GetAssetFullPath(LPCWSTR assetName);

    void GetHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);

private:
    // Root assets path.
    std::wstring assetsPath;
    // Adapter info.
    bool useWarpDevice;

    static const UINT FrameCount = 2;

    // Pipeline objects.
    ComPtr<IDXGISwapChain3> swapChain;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12Resource> renderTargets[FrameCount];
    ComPtr<ID3D12Resource> depthStencils[FrameCount];
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12CommandQueue> commandQueue;
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> dsvHeap;
    ComPtr<ID3D12DescriptorHeap> cbvHeap;
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12GraphicsCommandList> commandList;
    UINT rtvDescriptorSize;
    UINT dsvDescriptorSize;

    unique_ptr<D3D12BufferManager> allocator;
    unique_ptr<D3D12DescriptorHeapManager> descriptorHeapManager;

    // Synchronization objects.
    UINT frameIndex;
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue;

    // Scene objects
    shared_ptr<D3D12Camera> camera;
    shared_ptr<D3D12Model> model;
    unique_ptr<FBXImporter> fbxImporter;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};
