#pragma once

#include "Window.h"
#include "DrawObjecstPass.h"

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

    // Pipeline objects.
    shared_ptr<D3D12Device> pDevice;
    ComPtr<IDXGISwapChain3> pSwapChain;
    ComPtr<ID3D12Resource> renderTargets[FRAME_COUNT];
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    ComPtr<ID3D12RootSignature> rootSignature;
    ComPtr<ID3D12PipelineState> pipelineState;

    D3D12CommandList* pCommandList;
    shared_ptr<DrawObjectsPass> pDrawObjectPass;

    // Synchronization objects.
    UINT frameIndex;
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue;

    // Scene objects
    shared_ptr<SceneManager> pSceneManager;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
    void ExecuteCommandList();
    UINT64 UpdateFence();
};
