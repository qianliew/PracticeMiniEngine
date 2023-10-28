#include "stdafx.h"
#include "AbstractRenderPass.h"

AbstractRenderPass::AbstractRenderPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager) :
    pDevice(device),
    pSceneManager(sceneManager)
{

}

AbstractRenderPass::~AbstractRenderPass()
{

}
