#pragma once
#include "SceneManager.h"
#include "D3D12ShaderTable.h"

using namespace std;

#define SizeOfInUint32(obj) ((sizeof(obj) - 1) / sizeof(UINT32) + 1)

class RayTracingPass
{
private:
	const wchar_t* kHitGroupName = L"MyHitGroup";
	const wchar_t* kRaygenShaderName = L"MyRaygenShader";
	const wchar_t* kClosestHitShaderName = L"MyClosestHitShader";
	const wchar_t* kMissShaderName = L"MyMissShader";

	shared_ptr<D3D12Device> pDevice;
	shared_ptr<SceneManager> pSceneManager;
	ComPtr<ID3D12RootSignature> pRaytracingGlobalRootSignature;
	ComPtr<ID3D12RootSignature> pRaytracingLocalRootSignature;
	ComPtr<ID3D12StateObject> pDXRStateObject;
	ComPtr<ID3D12Resource> pMissShaderTable;
	ComPtr<ID3D12Resource> pHitGroupShaderTable;
	ComPtr<ID3D12Resource> pRayGenShaderTable;

public:
	RayTracingPass(shared_ptr<D3D12Device>&, shared_ptr<SceneManager>&);

	void Setup(D3D12CommandList*&, ComPtr<ID3D12RootSignature>&);
	void Execute(D3D12CommandList*&, UINT);
	void BuildShaderTables();
};
