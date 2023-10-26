#pragma once

#include "Window.h"
#include "FBXImporter.h"
#include "D3D12CommandList.h"
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

private:
    // Root assets path.
    std::wstring assetsPath;

    static const UINT FrameCount = 2;

    // Pipeline objects.
    unique_ptr<D3D12Device> pDevice;
    ComPtr<ID3D12Resource> renderTargets[FrameCount];
    ComPtr<ID3D12Resource> depthStencils[FrameCount];
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;
    ComPtr<ID3D12DescriptorHeap> rtvHeap;
    ComPtr<ID3D12DescriptorHeap> dsvHeap;
    UINT rtvDescriptorSize;
    UINT dsvDescriptorSize;

    unique_ptr<FBXImporter> fbxImporter;
    unique_ptr<D3D12CommandList> cmdList;

    // Synchronization objects.
    UINT frameIndex;
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue;

    // Scene objects
    std::vector<shared_ptr<D3D12Model>> models;
    shared_ptr<D3D12Camera> camera;
    shared_ptr<D3D12Model> model;
    shared_ptr<D3D12Model> model2;

    UINT id = 0;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
    void ExecuteCommandList();
    UINT64 UpdateFence();
};
