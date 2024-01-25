#pragma once
#include "D3D12Texture.h"

class AbstractMaterial
{
protected:
	std::wstring name;
	D3D12Texture* pTexture;

public:
	AbstractMaterial(const std::wstring inName);
	virtual ~AbstractMaterial();

	virtual void LoadTexture() = 0;
	virtual void ReleaseTextureData() = 0;

	inline D3D12Texture* GetTexture() const { return pTexture; }
};
