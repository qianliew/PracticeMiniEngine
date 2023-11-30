#pragma once

#include "Window.h"
#include "ViewManager.h"
#include "DrawObjecstPass.h"
#include "DrawSkyboxPass.h"
#include "BlitPass.h"
#include "TemporalAAPass.h"
#include "RayTracingPass.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::shared_ptr;

class MiniEngine : public Window
{
private:
    BOOL isDXR;

    // Pipeline objects.
    shared_ptr<D3D12Device> pDevice;
    D3D12RootSignature* pRootSignature;
    D3D12CommandList* pCommandList;

    shared_ptr<DrawObjectsPass> pDrawObjectPass;
    shared_ptr<DrawSkyboxPass> pDrawSkyboxPass;
    shared_ptr<TemporalAAPass> pTemporalAAPass;
    shared_ptr<BlitPass> pBlitPass;
    shared_ptr<RayTracingPass> pRayTracingPass;

    // Synchronization objects.
    HANDLE fenceEvent;
    ComPtr<ID3D12Fence> fence;
    UINT64 fenceValue;

    // Scene objects
    shared_ptr<SceneManager> pSceneManager;
    shared_ptr<ViewManager> pViewManager;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
    void WaitForGPU();
    UINT64 UpdateFence();

public:
    MiniEngine(UINT width, UINT height, std::wstring name);
    ~MiniEngine();

    static UINT sFrameCount;

    void OnInit();
    void OnUpdate();
    void OnRender();
    void OnDestroy();
    void OnKeyDown(UINT8 /*key*/);
    void OnKeyUp(UINT8 /*key*/);
};
