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

void DrawObjectsPass::Execute(D3D12CommandList*& pCommandList)
{
    pSceneManager->DrawObjects(pCommandList);
}
