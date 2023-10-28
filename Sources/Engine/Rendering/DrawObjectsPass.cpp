#include "stdafx.h"
#include "DrawObjecstPass.h"

DrawObjectsPass::DrawObjectsPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager) :
    AbstractRenderPass(device, sceneManager)
{

}

void DrawObjectsPass::Setup(D3D12CommandList*& pCommandList) 
{
    pSceneManager->CreateAndBindObjectBuffer(pCommandList);
}

void DrawObjectsPass::Execute(D3D12CommandList*& pCommandList, UINT frameIndex)
{
    pDevice->GetDescriptorHeapManager()->SetCBVs(pCommandList->GetCommandList(), CONSTANT_BUFFER_VIEW_GLOBAL, 0);
    pDevice->GetDescriptorHeapManager()->SetSRVs(pCommandList->GetCommandList());
    pDevice->GetDescriptorHeapManager()->SetSamplers(pCommandList->GetCommandList());

    // Set camera relating state.
    pCommandList->SetViewports(pSceneManager->GetCamera()->GetViewport());
    pCommandList->SetScissorRects(pSceneManager->GetCamera()->GetScissorRect());

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pDevice->GetDescriptorHeapManager()->GetRTVHandle(frameIndex);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = pDevice->GetDescriptorHeapManager()->GetDSVHandle(0);
    pCommandList->SetRenderTargets(1, &rtvHandle, &dsvHandle);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    pCommandList->ClearColor(rtvHandle, clearColor);
    pCommandList->ClearDepth(dsvHandle);

    pSceneManager->DrawObjects(pCommandList);
}
