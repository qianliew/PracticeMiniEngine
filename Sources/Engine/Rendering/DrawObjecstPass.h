#pragma once
#include "AbstractRenderPass.h"

class DrawObjectsPass : public AbstractRenderPass
{
public:
	DrawObjectsPass(shared_ptr<D3D12Device>&, shared_ptr<SceneManager>&);

	virtual void Setup(D3D12CommandList*&) override;
	virtual void Execute(D3D12CommandList*&, UINT) override;
};
