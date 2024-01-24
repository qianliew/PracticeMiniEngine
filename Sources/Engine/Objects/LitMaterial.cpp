#include "stdafx.h"
#include "LitMaterial.h"
#include "SceneManager.h"

LitMaterial::LitMaterial(std::wstring inName) :
	AbstractMaterial(inName)
{

}

LitMaterial::~LitMaterial()
{
	delete pTexture;
	delete pMRATexture;
	delete pNormalTexture;
}

void LitMaterial::LoadTexture()
{
	std::wstring texturePath = GetAssetPath(name.c_str());

	// Load the diffuse texture.
	UINT id = SceneManager::sTextureID++;
	pTexture = new D3D12Texture(id);
	pTexture->LoadTexture2D(texturePath, 1);
	pTexture->CreateTextureResource();

	// Load the MRA texture.
	id = SceneManager::sTextureID++;
	pMRATexture = new D3D12Texture(id);
	pMRATexture->LoadTexture2D(GetMRATexturePath(texturePath), 1);
	pMRATexture->CreateTextureResource();

	// Load the normal texture.
	id = SceneManager::sTextureID++;
	pNormalTexture = new D3D12Texture(id);
	pNormalTexture->LoadTexture2D(GetNormalTexturePath(texturePath), 1);
	pNormalTexture->CreateTextureResource();
}

void LitMaterial::ReleaseTextureData()
{
	if (pTexture != nullptr)
	{
		pTexture->ReleaseTextureData();
	}
	if (pMRATexture != nullptr)
	{
		pMRATexture->ReleaseTextureData();
	}
	if (pNormalTexture != nullptr)
	{
		pNormalTexture->ReleaseTextureData();
	}
}

std::wstring LitMaterial::GetMRATexturePath(std::wstring texturePath)
{
	std::wstring suffix = kMRASuffix;
	texturePath.insert(texturePath.find_last_of(L'.'), suffix);

	return texturePath;
}

std::wstring LitMaterial::GetNormalTexturePath(std::wstring texturePath)
{
	std::wstring suffix = kNormalSuffix;
	texturePath.insert(texturePath.find_last_of(L'.'), suffix);

	return texturePath;
}