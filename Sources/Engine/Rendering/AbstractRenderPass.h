#pragma once
#include "SceneManager.h"

class AbstractRenderPass
{
protected:
	shared_ptr<D3D12Device> pDevice;
	shared_ptr<SceneManager> pSceneManager;

public:
	AbstractRenderPass(shared_ptr<D3D12Device>&, shared_ptr<SceneManager>&);

	virtual void Setup(D3D12CommandList*&) = 0;
	virtual void Execute(D3D12CommandList*&) = 0;
};
