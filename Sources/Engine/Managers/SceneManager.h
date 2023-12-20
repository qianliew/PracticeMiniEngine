#pragma once
#include "FBXImporter.h"
#include "Camera.h"
#include "Model.h"
#include "AbstractMaterial.h"

struct BLAS
{
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;
	ComPtr<ID3D12Resource> pScratchResource;
	ComPtr<ID3D12Resource> pBottomLevelAccelerationStructure;
};

struct TLAS
{
	ComPtr<ID3D12Resource> pScratchResource;
	ComPtr<ID3D12Resource> pTopLevelAccelerationStructure;
};

class SceneManager
{
private:
	shared_ptr<D3D12Device> pDevice;
	unique_ptr<FBXImporter> pFBXImporter;

	std::vector<Model*> pObjects;
	std::map<wstring, AbstractMaterial*> pMaterialPool;
	UINT visData[GlobalConstants::kVisDataSize];
	AbstractMaterial* pSkyboxMaterial;
	Camera* pCamera;
	Model* pSkyboxMesh;
	Model* pFullScreenMesh;

	UINT objectID;

	// DXR member variables.
	BOOL isDXR;

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

public:
	SceneManager(shared_ptr<D3D12Device>&, BOOL isDXR);
	~SceneManager();

	static UINT sTextureID;

	void InitFBXImporter();
	void ParseScene(D3D12CommandList*);
	void LoadScene(D3D12CommandList*);
	void UnloadScene();
	void CreateCamera(UINT width, UINT height);
	void AddObject(Model* object);
	void DrawObjects(D3D12CommandList*);
	void DrawSkybox(D3D12CommandList*);
	void DrawFullScreenMesh(D3D12CommandList*);
	void SetFrustumCullingResources(D3D12CommandList*);
	void SetDXRResources(D3D12CommandList*);

	void UpdateScene();
	void UpdateTransforms();
	void UpdateCamera();

	void Release();

	inline const std::vector<Model*>& GetObjects() const { return pObjects; }
	inline UINT* GetVisData() { return visData; }
	inline Camera* GetCamera() const { return pCamera; }
	inline Model* GetSkybox() const { return pSkyboxMesh; }
};
