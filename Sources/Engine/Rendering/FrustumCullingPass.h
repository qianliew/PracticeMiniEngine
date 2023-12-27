#pragma once
#include "AbstractRenderPass.h"
#include "D3D12ShaderTable.h"

class FrustumCullingPass : public AbstractRenderPass
{
private:
	// Main
	const wchar_t* kHitGroupName = L"FrustumCullingHitGroup";
	const wchar_t* kRaygenShaderName = L"FrustumCullingRaygenShader";
	const wchar_t* kIntersectionShaderName = L"FrustumCullingIntersectionShader";
	const wchar_t* kClosestHitShaderName = L"FrustumCullingClosestHitShader";
	const wchar_t* kMissShaderName = L"FrustumCullingMissShader";

	ComPtr<ID3D12RootSignature> pRaytracingLocalRootSignature;
	ComPtr<ID3D12StateObject> pDXRStateObject;
	ComPtr<ID3D12Resource> pMissShaderTable;
	ComPtr<ID3D12Resource> pHitGroupShaderTable;
	ComPtr<ID3D12Resource> pRayGenShaderTable;

	D3D12UnorderedAccessBuffer* pFrustumCullingData;
	UINT visData[GlobalConstants::kVisDataSize];

public:
	FrustumCullingPass(shared_ptr<D3D12Device>&, shared_ptr<SceneManager>&, shared_ptr<ViewManager>&);

	void Setup(D3D12CommandList*, ComPtr<ID3D12RootSignature>&);
	void Update();
	void Execute(D3D12CommandList*);
};
