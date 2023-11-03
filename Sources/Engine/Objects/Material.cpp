#include "stdafx.h"
#include "Material.h"

Material::Material(std::wstring inName) :
	name(inName)
{

}

Material::~Material()
{

}

void Material::LoadTexture(UINT& textureID)
{
	// Create Diffuse texture
	UINT id = textureID++;
	pTexture = new D3D12Texture(id);
	pTexture->LoadTexture(GetAssetPath(name.c_str()));
	pTexture->CreateTexture(D3D12TextureType::ShaderResource, TRUE);
}
