#pragma once
#include "D3D12Texture.h"

class Material
{
private:
	// TODO: Parse the shader to decide how many textures in each material.
	// Material is equal to Shader in this engine right now, and there is just one object shader.
	const UINT kTextureNum = 1;
	std::wstring name;

	D3D12Texture* pTexture;

public:
	Material(std::wstring inName);
	~Material();

	void LoadTexture(UINT& textureID);

	inline const UINT GetTextureNum() const { return kTextureNum; }
	inline D3D12Texture* GetTexture() const { return pTexture; }
};
