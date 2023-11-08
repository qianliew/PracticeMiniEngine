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
	pTexture->LoadTexture(texturePath);
	pTexture->CreateTexture(D3D12TextureType::ShaderResource, TRUE);

	// Load the MRA texture.
	id = SceneManager::sTextureID++;
	pMRATexture = new D3D12Texture(id);
	pMRATexture->LoadTexture(GetMRATexturePath(texturePath));
	pMRATexture->CreateTexture(D3D12TextureType::ShaderResource, TRUE);

	// Load the normal texture.
	id = SceneManager::sTextureID++;
	pNormalTexture = new D3D12Texture(id);
	pNormalTexture->LoadTexture(GetNormalTexturePath(texturePath));
	pNormalTexture->CreateTexture(D3D12TextureType::ShaderResource, TRUE);
}

void LitMaterial::ReleaseTextureData()
{
	pTexture->ReleaseTextureData();
	pMRATexture->ReleaseTextureData();
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