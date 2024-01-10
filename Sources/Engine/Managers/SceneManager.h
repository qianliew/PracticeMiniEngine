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

enum class eCommandSignatureIndex
{
	CBV = 0,
};

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

	// Frustum Culling data.
	D3D12UnorderedAccessBuffer* pFrustumCullingData;
	D3D12UploadBuffer* pUploadBuffer;
	D3D12ReadbackBuffer* pReadbackBuffer;
	UINT visData[GlobalConstants::kVisDataSize];

	std::shared_ptr<D3D12ShaderResourceBuffer> pCommandBuffer;
	D3D12UploadBuffer* pTempCommandBuffer;
	std::shared_ptr<D3D12UnorderedAccessBuffer> pProcessedCommandBuffer;
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
	void LoadObjectVertexBufferAndIndexBuffer(D3D12CommandList*, Model* object);
	void LoadObjectVertexBufferAndIndexBufferDXR(D3D12CommandList*, Model* object, UINT& offset);
	void LoadTextureBufferAndSampler(D3D12CommandList*, D3D12Texture* texture);
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
	void DrawObjects(D3D12CommandList*);
	void DrawSkybox(D3D12CommandList*);
	void DrawFullScreenMesh(D3D12CommandList*);
	void SetFrustumCullingResources(D3D12CommandList*);
	void ReadbackFrustumCullingData(D3D12CommandList*);
	void SetDXRResources(D3D12CommandList*);

	void UpdateScene();
	void UpdateTransforms();
	void UpdateCamera();

	void Release();

	inline const std::vector<Model*>& GetObjects() const { return pObjects; }
	inline Camera* GetCamera() const { return pCamera; }
	inline Model* GetSkybox() const { return pSkyboxMesh; }
};
