#include "stdafx.h"
#include "IndirectDrawingMaterial.h"
#include "SceneManager.h"

IndirectDrawingMaterial::IndirectDrawingMaterial() :
	LitMaterial(nullptr)
{

}

IndirectDrawingMaterial::~IndirectDrawingMaterial()
{

}

void IndirectDrawingMaterial::AddTextures(const std::wstring inName)
{
	names.push_back(inName);
	mraNames.push_back(GetMRATexturePath(inName));
	normalNames.push_back(GetNormalTexturePath(inName));
}

void IndirectDrawingMaterial::LoadTexture()
{
	// Load the diffuse texture.
	UINT id = SceneManager::sTextureID++;
	pTexture = new D3D12Texture(id);
	pTexture->LoadTexture2DArray(names, 1);
	pTexture->CreateTextureResource();

	// Load the MRA texture.
	id = SceneManager::sTextureID++;
	pMRATexture = new D3D12Texture(id);
	pMRATexture->LoadTexture2DArray(mraNames, 1);
	pMRATexture->CreateTextureResource();

	// Load the normal texture.
	id = SceneManager::sTextureID++;
	pNormalTexture = new D3D12Texture(id);
	pNormalTexture->LoadTexture2DArray(normalNames, 1);
	pNormalTexture->CreateTextureResource();
}
