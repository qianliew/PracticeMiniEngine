#include "stdafx.h"
#include "MiniEngine.h"

using namespace Microsoft::WRL;

MiniEngine::MiniEngine(UINT width, UINT height, std::wstring name) :
    Window(width, height, name),
    frameIndex(0)
{

}

MiniEngine::~MiniEngine()
{
    delete pCommandList;
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
    frameIndex = pViewManager->GetSwapChain()->GetCurrentBackBufferIndex();

    // Create the command allocator.
    ThrowIfFailed(pDevice->GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));

    // Create the command list.
    pCommandList = new D3D12CommandList(pDevice->GetDevice(), commandAllocator);
}

// Load the sample assets.
void MiniEngine::LoadAssets()
{
    // Create an empty root signature.
    {
        CD3DX12_DESCRIPTOR_RANGE descriptorTableRanges[2];
        descriptorTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);
        descriptorTableRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 0, 0);
        // descriptorTableRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0);
        // descriptorTableRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0);

        CD3DX12_ROOT_PARAMETER rootParameters[4];
        rootParameters[CONSTANT_BUFFER_VIEW_GLOBAL].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[CONSTANT_BUFFER_VIEW_PEROBJECT].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters[SHADER_RESOURCE_VIEW].InitAsDescriptorTable(1, &descriptorTableRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
        rootParameters[SAMPLER].InitAsDescriptorTable(1, &descriptorTableRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);

        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            signature.GetAddressOf(),
            error.GetAddressOf()));
        ThrowIfFailed(pDevice->GetDevice()->CreateRootSignature(0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&rootSignature)));
    }

    // Create scene objects.
    {
        // Create and init the scene manager.
        pSceneManager = make_shared<SceneManager>(pDevice);
        pSceneManager->InitFBXImporter();
        pSceneManager->LoadScene(pCommandList);
        pSceneManager->CreateCamera(width, height);

        // Create and init render passes.
        pDrawObjectPass = make_shared<DrawObjectsPass>(pDevice, pSceneManager);
        pDrawObjectPass->Setup(pCommandList, rootSignature);

        pBlitPass = make_shared<BlitPass>(pDevice, pSceneManager);
        pBlitPass->Setup(pCommandList, rootSignature);
    }

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(pDevice->GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        ExecuteCommandList();
        UpdateFence();

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }
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
    pSceneManager->UpdateTransforms();
    pSceneManager->UpdateCamera();
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
    ThrowIfFailed(commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    pCommandList->Reset(commandAllocator);
    pCommandList->SetRootSignature(rootSignature);

    // Indicate that the back buffer will be used as a render target.
    pCommandList->AddTransitionResourceBarriers(pViewManager->GetBackBufferAt(frameIndex),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    pCommandList->FlushResourceBarriers();

    pDrawObjectPass->Execute(pCommandList, frameIndex);

    pViewManager->EmplaceRenderTarget(pCommandList, D3D12TextureType::ShaderResource);
    pBlitPass->Execute(pCommandList, frameIndex);
    pViewManager->EmplaceRenderTarget(pCommandList, D3D12TextureType::RenderTarget);

    // Indicate that the back buffer will now be used to present.
    pCommandList->AddTransitionResourceBarriers(pViewManager->GetBackBufferAt(frameIndex),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    pCommandList->FlushResourceBarriers();

    ExecuteCommandList();
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

    frameIndex = pViewManager->GetSwapChain()->GetCurrentBackBufferIndex();

    // Release upload buffers from last frame.
    pDevice->GetBufferManager()->ReleaseUploadBuffer();
    // TODO: Add a event system to handle event.
    pSceneManager->Release();
}

void MiniEngine::ExecuteCommandList()
{
    ThrowIfFailed(pCommandList->GetCommandList()->Close());

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { pCommandList->GetCommandList().Get() };
    pDevice->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

UINT64 MiniEngine::UpdateFence()
{
    const UINT64 value = fenceValue;
    ThrowIfFailed(pDevice->GetCommandQueue()->Signal(fence.Get(), value));
    fenceValue++;

    return value;
}
