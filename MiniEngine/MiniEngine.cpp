#include "stdafx.h"
#include "MiniEngine.h"

using namespace Microsoft::WRL;

MiniEngine::MiniEngine(UINT width, UINT height, std::wstring name) :
    Window(width, height, name)
{

}

MiniEngine::~MiniEngine()
{
    delete pCommandList;
    delete pRootSignature;
}

void MiniEngine::OnInit()
{
    LoadPipeline();
    LoadAssets();
}

// Load the rendering pipeline dependencies.
void MiniEngine::LoadPipeline()
{
    // Create the device.
    pDevice = std::make_shared<D3D12Device>();
    pDevice->CreateDevice();
    pDevice->CreateDescriptorHeapManager();
    pDevice->CreateBufferManager();

    // Create and init the view manager.
    pViewManager = make_shared<ViewManager>(pDevice, width, height);

    // Create the command list.
    pCommandList = new D3D12CommandList(pDevice);

    // Create the root signature.
    pRootSignature = new D3D12RootSignature(pDevice);
    pRootSignature->CreateRootSignature();
    pRootSignature->CreateDXRRootSignature();
}

// Load the sample assets.
void MiniEngine::LoadAssets()
{
    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    ThrowIfFailed(pDevice->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    fenceValue = 1;

    // Create an event handle to use for frame synchronization.
    fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    // Create scene objects.
    pSceneManager = make_shared<SceneManager>(pDevice);
    pSceneManager->InitFBXImporter();
    pSceneManager->LoadScene(pCommandList);
    pSceneManager->CreateCamera(width, height);
    pCommandList->ExecuteCommandList();
    WaitForGPU();

    // Create and init render passes.
    pCommandList->Reset(pDevice->GetCommandAllocator());

    pRayTracingPass = make_shared<RayTracingPass>(pDevice, pSceneManager, pViewManager);
    pRayTracingPass->Setup(pCommandList, pRootSignature->GetDRXRootSignature());
    pRayTracingPass->BuildShaderTables();

    pFrustumCullingPass = make_shared<FrustumCullingPass>(pDevice, pSceneManager, pViewManager);
    pFrustumCullingPass->Setup(pCommandList, pRootSignature->GetDRXRootSignature());

    pDrawObjectPass = make_shared<DrawObjectsPass>(pDevice, pSceneManager, pViewManager);
    pDrawObjectPass->Setup(pCommandList, pRootSignature->GetRootSignature());

    pGBufferPass = make_shared<GBufferPass>(pDevice, pSceneManager, pViewManager);
    pGBufferPass->Setup(pCommandList, pRootSignature->GetRootSignature());

    pDeferredLightingPass = make_shared<DeferredLightingPass>(pDevice, pSceneManager, pViewManager);
    pDeferredLightingPass->Setup(pCommandList, pRootSignature->GetRootSignature());

    pDrawSkyboxPass = make_shared<DrawSkyboxPass>(pDevice, pSceneManager, pViewManager);
    pDrawSkyboxPass->Setup(pCommandList, pRootSignature->GetRootSignature());

    pTemporalAAPass = make_shared<TemporalAAPass>(pDevice, pSceneManager, pViewManager);
    pTemporalAAPass->Setup(pCommandList, pRootSignature->GetRootSignature());

    pBlitPass = make_shared<BlitPass>(pDevice, pSceneManager, pViewManager);
    pBlitPass->Setup(pCommandList, pRootSignature->GetRootSignature());

    pCommandList->ExecuteCommandList();
    WaitForGPU();
}

void MiniEngine::OnKeyDown(UINT8 key)
{
    switch (key)
    {
    case 'A':
        pSceneManager->GetCamera()->MoveAlongX(1);
        break;
    case 'D':
        pSceneManager->GetCamera()->MoveAlongX(-1);
        break;
    case 'W':
        pSceneManager->GetCamera()->MoveAlongZ(1);
        break;
    case 'S':
        pSceneManager->GetCamera()->MoveAlongZ(-1);
        break;

    case 'Q':
        pSceneManager->GetCamera()->RotateAlongY(1);
        break;
    case 'E':
        pSceneManager->GetCamera()->RotateAlongY(-1);
        break;
    case 'Z':
        pSceneManager->GetCamera()->RotateAlongX(1);
        break;
    case 'X':
        pSceneManager->GetCamera()->RotateAlongX(-1);
        break;

    case 'C':
        pSceneManager->GetCamera()->ResetTransform();
        break;
    }
}

void MiniEngine::OnKeyUp(UINT8 key)
{

}

// Update frame-based values.
void MiniEngine::OnUpdate()
{
    // Update scene objects.
    pSceneManager->UpdateScene();
    pSceneManager->UpdateTransforms();
    pSceneManager->UpdateCamera();
    pFrustumCullingPass->Update();
}

// Render the scene.
void MiniEngine::OnRender()
{
    // Record all the commands we need to render the scene into the command list.
    PopulateCommandList();

    // Present the frame.
    ThrowIfFailed(pViewManager->GetSwapChain()->Present(1, 0));

    WaitForPreviousFrame();
}

void MiniEngine::OnDestroy()
{
    // Ensure that the GPU is no longer referencing resources that are about to be
    // cleaned up by the destructor.
    WaitForPreviousFrame();

    CloseHandle(fenceEvent);
}

void MiniEngine::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(pDevice->GetCommandAllocator()->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    pCommandList->Reset(pDevice->GetCommandAllocator());

    // Indicate that the back buffer will be used as a render target.
    pCommandList->AddTransitionResourceBarriers(pViewManager->GetCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    pCommandList->FlushResourceBarriers();

    pCommandList->SetComputeRootSignature(pRootSignature->GetDRXRootSignature());
    pCommandList->SetComputeRootConstantBufferView(
        (UINT)eDXRRootIndex::ConstantBufferViewGlobal,
        pDevice->GetBufferManager()->GetGlobalConstantBuffer()->GetResource()->GetGPUVirtualAddress());
    pFrustumCullingPass->Execute(pCommandList);

    pCommandList->SetRootSignature(pRootSignature->GetRootSignature());
    pCommandList->SetRootConstantBufferView(
        (UINT)eRootIndex::ConstantBufferViewGlobal,
        pDevice->GetBufferManager()->GetGlobalConstantBuffer()->GetResource()->GetGPUVirtualAddress());
    pGBufferPass->Execute(pCommandList);

    pCommandList->SetComputeRootSignature(pRootSignature->GetRootSignature());
    pCommandList->SetComputeRootConstantBufferView(
        (UINT)eRootIndex::ConstantBufferViewGlobal,
        pDevice->GetBufferManager()->GetGlobalConstantBuffer()->GetResource()->GetGPUVirtualAddress());
    pDeferredLightingPass->Execute(pCommandList);

    pCommandList->SetComputeRootSignature(pRootSignature->GetDRXRootSignature());
    pRayTracingPass->Execute(pCommandList);

    pCommandList->SetRootSignature(pRootSignature->GetRootSignature());
    pTemporalAAPass->Execute(pCommandList);
    pBlitPass->Execute(pCommandList);

    // Indicate that the back buffer will now be used to present.
    pCommandList->AddTransitionResourceBarriers(pViewManager->GetCurrentBackBuffer(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    pCommandList->FlushResourceBarriers();

    pCommandList->ExecuteCommandList();
}

void MiniEngine::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 value = UpdateFence();

    // Wait until the previous frame is finished.
    if (fence->GetCompletedValue() < value)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(value, fenceEvent));
        WaitForSingleObject(fenceEvent, INFINITE);
    }

    // Update the frame index.
    pViewManager->UpdateFrameIndex();

    // Release upload buffers from last frame.
    pDevice->GetBufferManager()->ReleaseTempUploadBuffer();
    // TODO: Add a event system to handle event.
    pSceneManager->Release();
}

// Wait for pending GPU work to complete.
void MiniEngine::WaitForGPU()
{
    const UINT64 value = UpdateFence();

    // Wait until the previous frame is finished.
    if (fence->GetCompletedValue() < value)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(value, fenceEvent));
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

UINT64 MiniEngine::UpdateFence()
{
    const UINT64 value = fenceValue;
    ThrowIfFailed(pDevice->GetCommandQueue()->Signal(fence.Get(), value));
    fenceValue++;

    return value;
}
