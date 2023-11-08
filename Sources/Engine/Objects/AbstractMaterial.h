#pragma once
#include "D3D12Texture.h"

class AbstractMaterial
{
protected:
	std::wstring name;

public:
	AbstractMaterial(std::wstring inName);
	virtual ~AbstractMaterial();

	virtual void LoadTexture() = 0;
	virtual void ReleaseTextureData() = 0;
};
