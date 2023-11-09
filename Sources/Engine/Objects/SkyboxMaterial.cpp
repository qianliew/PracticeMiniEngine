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
	pTexture->LoadTexture(texturePath, 1, D3D12_SRV_DIMENSION_TEXTURECUBE, 6);
	pTexture->CreateTexture(D3D12TextureType::ShaderResource);
}

void SkyboxMaterial::ReleaseTextureData()
{
	pTexture->ReleaseTextureData();
}
