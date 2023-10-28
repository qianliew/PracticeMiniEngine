#pragma once
#include "SceneManager.h"

class AbstractRenderPass
{
protected:
	shared_ptr<D3D12Device> pDevice;
	shared_ptr<SceneManager> pSceneManager;

	D3D12Texture* pDepthStencil;

public:
	AbstractRenderPass(shared_ptr<D3D12Device>&, shared_ptr<SceneManager>&);
	~AbstractRenderPass();

	virtual void Setup(D3D12CommandList*&) = 0;
	virtual void Execute(D3D12CommandList*&, UINT) = 0;
};
