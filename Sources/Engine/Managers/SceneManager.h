#pragma once
#include "FBXImporter.h"
#include "D3D12Texture.h"
#include "Camera.h"
#include "Model.h"

class SceneManager
{
private:
	shared_ptr<D3D12Device> pDevice;
	unique_ptr<FBXImporter> pFBXImporter;

	std::vector<Model*> pObjects;
	std::map<LPCWSTR, D3D12Texture*> pTextures;
	Camera* pCamera;
	Model* pFullScreenMesh;

	UINT objectID;
	UINT textureID;

	// Helper functions.
	void LoadObjectVertexBufferAndIndexBuffer(D3D12CommandList*&, Model* object);
	void LoadTextureBufferAndSampler(D3D12CommandList*&, D3D12Texture* texture);

public:
	SceneManager(shared_ptr<D3D12Device>&);
	~SceneManager();

	void InitFBXImporter();
	void ParseScene(D3D12CommandList*&);
	void LoadScene(D3D12CommandList*&);
	void UnloadScene();
	void CreateCamera(UINT width, UINT height);
	void AddObject(Model* object);
	void DrawObjects(D3D12CommandList*&);
	void DrawFullScreenMesh(D3D12CommandList*&);

	void UpdateTransforms();
	void UpdateCamera();

	inline const std::vector<Model*>& GetObjects() const { return pObjects; }
	inline Camera* GetCamera() const { return pCamera; }
};
