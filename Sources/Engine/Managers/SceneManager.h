#pragma once
#include "FBXImporter.h"
#include "D3D12Model.h"
#include "D3D12Camera.h"

class SceneManager
{
private:
	shared_ptr<D3D12Device> pDevice;
	unique_ptr<FBXImporter> pFBXImporter;

	std::vector<D3D12Model*> pObjects;
	D3D12Camera* pCamera;
	D3D12Model* pFullScreenMesh;

	UINT objectID;

	// Helper functions.
	void LoadObjectVertexBufferAndIndexBuffer(D3D12CommandList*&, D3D12Model* object);

public:
	SceneManager(shared_ptr<D3D12Device>&);
	~SceneManager();

	void InitFBXImporter();
	void LoadScene(D3D12CommandList*&);
	void UnloadScene();
	void CreateCamera(UINT width, UINT height);
	void AddObject(D3D12Model* object);
	void CreateAndBindObjectBuffer(D3D12CommandList*&);
	void DrawObjects(D3D12CommandList*&);
	void DrawFullScreenMesh(D3D12CommandList*&);

	void UpdateTransforms();
	void UpdateCamera();

	inline const std::vector<D3D12Model*>& GetObjects() const { return pObjects; }
	inline D3D12Camera* GetCamera() const { return pCamera; }
};
