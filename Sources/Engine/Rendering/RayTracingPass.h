#pragma once
#include "SceneManager.h"
#include "ViewManager.h"
#include "D3D12ShaderTable.h"

using namespace std;

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

class RayTracingPass
{
private:
	// Main
	const wchar_t* kHitGroupName = L"HitGroup";
	const wchar_t* kRaygenShaderName = L"RaygenShader";
	const wchar_t* kClosestHitShaderName = L"ClosestHitShader";
	const wchar_t* kMissShaderName = L"MissShader";

	// AO
	const wchar_t* kAOHitGroupName = L"AOHitGroup";
	const wchar_t* kAOClosestHitShaderName = L"AOClosestHitShader";
	const wchar_t* kAOMissShaderName = L"AOMissShader";

	// GI
	const wchar_t* kGIHitGroupName = L"GIHitGroup";
	const wchar_t* kGIClosestHitShaderName = L"GIClosestHitShader";
	const wchar_t* kGIMissShaderName = L"GIMissShader";

	shared_ptr<D3D12Device> pDevice;
	shared_ptr<SceneManager> pSceneManager;
	shared_ptr<ViewManager> pViewManager;
	ComPtr<ID3D12RootSignature> pRaytracingLocalRootSignature;
	ComPtr<ID3D12StateObject> pDXRStateObject;
	ComPtr<ID3D12Resource> pMissShaderTable;
	ComPtr<ID3D12Resource> pHitGroupShaderTable;
	ComPtr<ID3D12Resource> pRayGenShaderTable;

public:
	RayTracingPass(shared_ptr<D3D12Device>&, shared_ptr<SceneManager>&, shared_ptr<ViewManager>&);

	void Setup(D3D12CommandList*, ComPtr<ID3D12RootSignature>&);
	void Execute(D3D12CommandList*);
	void BuildShaderTables();
};
