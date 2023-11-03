#include "stdafx.h"
#include "Material.h"

Material::Material(std::wstring inName) :
	name(inName)
{

}

Material::~Material()
{
	delete pTexture;
	delete pMRATexture;
	delete pNormalTexture;
}

void Material::LoadTexture(UINT& textureID)
{
	std::wstring texturePath = GetAssetPath(name.c_str());

	// Load the diffuse texture.
	UINT id = textureID++;
	pTexture = new D3D12Texture(id);
	pTexture->LoadTexture(texturePath);
	pTexture->CreateTexture(D3D12TextureType::ShaderResource, TRUE);

	// Load the MRA texture.
	id = textureID++;
	pMRATexture = new D3D12Texture(id);
	pMRATexture->LoadTexture(GetMRATexturePath(texturePath));
	pMRATexture->CreateTexture(D3D12TextureType::ShaderResource, TRUE);

	// Load the normal texture.
	id = textureID++;
	pNormalTexture = new D3D12Texture(id);
	pNormalTexture->LoadTexture(GetNormalTexturePath(texturePath));
	pNormalTexture->CreateTexture(D3D12TextureType::ShaderResource, TRUE);
}

void Material::ReleaseTextureData()
{
	pTexture->ReleaseTextureData();
	pMRATexture->ReleaseTextureData();
}

std::wstring Material::GetMRATexturePath(std::wstring texturePath)
{
	std::wstring suffix = kMRASuffix;
	texturePath.insert(texturePath.find_last_of(L'.'), suffix);

	return texturePath;
}

std::wstring Material::GetNormalTexturePath(std::wstring texturePath)
{
	std::wstring suffix = kNormalSuffix;
	texturePath.insert(texturePath.find_last_of(L'.'), suffix);

	return texturePath;
}