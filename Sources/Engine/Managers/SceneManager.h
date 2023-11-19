#pragma once
#include "FBXImporter.h"
#include "Camera.h"
#include "Model.h"
#include "AbstractMaterial.h"

class SceneManager
{
private:
	shared_ptr<D3D12Device> pDevice;
	unique_ptr<FBXImporter> pFBXImporter;

	std::vector<Model*> pObjects;
	std::map<wstring, AbstractMaterial*> pMaterialPool;
	AbstractMaterial* pSkyboxMaterial;
	Camera* pCamera;
	Model* pSkyboxMesh;
	Model* pFullScreenMesh;

	UINT objectID;
	BOOL isDXR;

	ComPtr<ID3D12Resource> pScratchResource;
	ComPtr<ID3D12Resource> pBottomLevelAccelerationStructure;
	ComPtr<ID3D12Resource> pTopLevelAccelerationStructure;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc;

	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;
	D3D12UploadBuffer* pInstanceDescBuffer;

	// Helper functions.
	void LoadObjectVertexBufferAndIndexBuffer(D3D12CommandList*&, Model* object);
	void LoadTextureBufferAndSampler(D3D12CommandList*&, D3D12Texture* texture);

public:
	SceneManager(shared_ptr<D3D12Device>&, BOOL isDXR);
	~SceneManager();

	static UINT sTextureID;

	void InitFBXImporter();
	void ParseScene(D3D12CommandList*&);
	void LoadScene(D3D12CommandList*&);
	void UnloadScene();
	void CreateCamera(UINT width, UINT height);
	void AddObject(Model* object);
	void DrawObjects(D3D12CommandList*&);
	void DrawSkybox(D3D12CommandList*&);
	void DrawFullScreenMesh(D3D12CommandList*&);

	void UpdateTransforms();
	void UpdateCamera();

	void Release();

	inline const std::vector<Model*>& GetObjects() const { return pObjects; }
	inline Camera* GetCamera() const { return pCamera; }
	inline Model* GetSkybox() const { return pSkyboxMesh; }

	inline ComPtr<ID3D12Resource>& GetTLAS() { return pTopLevelAccelerationStructure; }
};
