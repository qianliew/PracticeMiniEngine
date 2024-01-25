#pragma once
#include "FBXImporter.h"
#include "Camera.h"
#include "Model.h"
#include "AbstractMaterial.h"

struct BLAS
{
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;
	std::shared_ptr<D3D12UnorderedAccessBuffer> pScratchResource;
	std::shared_ptr<D3D12UnorderedAccessBuffer> pBottomLevelAccelerationStructure;
};

struct TLAS
{
	std::shared_ptr<D3D12UnorderedAccessBuffer> pScratchResource;
	std::shared_ptr<D3D12UnorderedAccessBuffer> pTopLevelAccelerationStructure;
};

class SceneManager
{
private:
	shared_ptr<D3D12Device> pDevice;
	unique_ptr<FBXImporter> pFBXImporter;

	std::vector<Model*> pObjects;
	std::map<wstring, AbstractMaterial*> pMaterialPool;
	AbstractMaterial* pSkyboxMaterial;
	AbstractMaterial* pIndirectDrawingMaterial;
	Camera* pCamera;
	Model* pSkyboxMesh;
	Model* pFullScreenMesh;

	UINT objectID;

	// Frustum Culling data.
	D3D12UnorderedAccessBuffer* pFrustumCullingData;
	D3D12UploadBuffer* pUploadBuffer;
	D3D12ReadbackBuffer* pReadbackBuffer;
	UINT visData[GlobalConstants::kVisDataSize];

	const UINT64 commandBufferSize = GlobalConstants::kVisDataSize * sizeof(IndirectCommand);
	const UINT64 argumentBufferSize = Align(commandBufferSize, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);

	std::shared_ptr<D3D12ShaderResourceBuffer> pCommandBuffer;
	D3D12UploadBuffer* pTempCommandBuffer;
	std::shared_ptr<D3D12UnorderedAccessBuffer> pArgumentBuffer;
	D3D12UploadBuffer* pCountBuffer;
	ComPtr<ID3D12CommandSignature> pCommandSignature;

	// DXR member variables.
	BLAS blas[GeometryType::Count];
	TLAS tlas[GeometryType::Count];

	D3D12ShaderResourceBuffer* pVertexBuffer;
	D3D12ShaderResourceBuffer* pIndexBuffer;
	D3D12ShaderResourceBuffer* pOffsetBuffer;

	D3D12UploadBuffer* pTempVertexBuffer;
	D3D12UploadBuffer* pTempIndexBuffer;
	D3D12UploadBuffer* pTempOffsetBuffer;
	D3D12UploadBuffer* pTempBoundingBoxBuffer;
	D3D12UploadBuffer* pInstanceDescBuffer;

	// Helper functions.
	void LoadObjectVertexBufferAndIndexBuffer(D3D12CommandList* pCommandList, Model* object);
	void LoadObjectVertexBufferAndIndexBufferDXR(D3D12CommandList* pCommandList, Model* object, UINT& indexOffset, UINT& vertexOffset);
	void LoadTextureBufferAndSampler(D3D12CommandList* pCommandList, D3D12Texture* texture);
	void BuildBottomLevelAS(D3D12CommandList* pCommandList, UINT index);
	void BuildTopLevelAS(D3D12CommandList* pCommandList, UINT index);

	void ResetVisData(D3D12CommandList* pCommandList);

public:
	SceneManager() = delete;
	SceneManager(shared_ptr<D3D12Device>&);
	~SceneManager();

	static UINT sTextureID;

	void InitFBXImporter();
	void ParseScene(D3D12CommandList*);
	void LoadScene(D3D12CommandList*, ComPtr<ID3D12RootSignature>&);
	void UnloadScene();
	void CreateCamera(UINT width, UINT height);
	void AddObject(Model* object);
	void DrawObjects(D3D12CommandList* pCommandList);
	void DrawObjectsIndirectly(D3D12CommandList* pCommandList);
	void DrawSkybox(D3D12CommandList* pCommandList);
	void DrawFullScreenMesh(D3D12CommandList* pCommandList);
	void SetFrustumCullingResources(D3D12CommandList* pCommandList);
	void SetIndirectDrawingResources(D3D12CommandList* pCommandList);
	void ReadbackFrustumCullingData(D3D12CommandList* pCommandList);
	void SetDXRResources(D3D12CommandList*);

	void UpdateScene();
	void UpdateTransforms();
	void UpdateCamera();

	void Release();

	inline const std::vector<Model*>& GetObjects() const { return pObjects; }
	inline Camera* GetCamera() const { return pCamera; }
	inline Model* GetSkybox() const { return pSkyboxMesh; }
};
