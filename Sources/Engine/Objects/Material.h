#pragma once
#include "D3D12Texture.h"

class Material
{
private:
	// TODO: Parse the shader to decide how many textures in each material.
	// Material is equal to Shader in this engine right now, and there is just one object shader.
	const UINT kTextureNum = 2;
	const std::wstring kMRASuffix = L"_mra";
	const std::wstring kNormalSuffix = L"_n";
	std::wstring name;

	D3D12Texture* pTexture;
	D3D12Texture* pMRATexture;
	D3D12Texture* pNormalTexture;

	//Helper functions
	std::wstring GetMRATexturePath(std::wstring texturePath);
	std::wstring GetNormalTexturePath(std::wstring texturePath);

public:
	Material(std::wstring inName);
	~Material();

	void LoadTexture(UINT& textureID);
	void ReleaseTextureData();

	inline const UINT GetTextureNum() const { return kTextureNum; }
	inline D3D12Texture* GetTexture() const { return pTexture; }
	inline D3D12Texture* GetMRATexture() const { return pMRATexture; }
	inline D3D12Texture* GetNormalTexture() const { return pNormalTexture; }
};
