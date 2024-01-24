#include "stdafx.h"
#include "SkyboxMaterial.h"
#include "SceneManager.h"

SkyboxMaterial::SkyboxMaterial(std::wstring inName) :
	AbstractMaterial(inName)
{

}

SkyboxMaterial::~SkyboxMaterial()
{
	delete pTexture;
}


void SkyboxMaterial::LoadTexture()
{
	std::wstring texturePath = GetAssetPath(name.c_str());

	// Load the diffuse texture.
	UINT id = SceneManager::sTextureID++;
	pTexture = new D3D12Texture(id);
	pTexture->LoadTextureCube(texturePath, 1);
	pTexture->CreateTextureResource();
}

void SkyboxMaterial::ReleaseTextureData()
{
	pTexture->ReleaseTextureData();
}
