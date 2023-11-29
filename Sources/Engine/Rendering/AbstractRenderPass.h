#pragma once
#include "SceneManager.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

class AbstractRenderPass
{
protected:
	shared_ptr<D3D12Device> pDevice;
	shared_ptr<SceneManager> pSceneManager;

	ComPtr<ID3D12PipelineState> pPipelineState;

public:
	AbstractRenderPass(shared_ptr<D3D12Device>&, shared_ptr<SceneManager>&);
	~AbstractRenderPass();

	virtual void Setup(D3D12CommandList*&, ComPtr<ID3D12RootSignature>&) = 0;
	virtual void Execute(D3D12CommandList*&, UINT) = 0;
};
