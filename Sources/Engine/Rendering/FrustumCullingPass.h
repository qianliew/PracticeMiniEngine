#pragma once
#include "AbstractRenderPass.h"

class FrustumCullingPass : public AbstractRenderPass
{
private:

public:
	FrustumCullingPass(shared_ptr<D3D12Device>&, shared_ptr<SceneManager>&, shared_ptr<ViewManager>&);

	void Setup(D3D12CommandList*, ComPtr<ID3D12RootSignature>&);
	void Execute(D3D12CommandList*);
};
