#pragma once
#include "AbstractMaterial.h"

class SkyboxMaterial : public AbstractMaterial
{
private:
	D3D12Texture* pTexture;

public:
	SkyboxMaterial(std::wstring inName);
	~SkyboxMaterial();

	virtual void LoadTexture() override;
	virtual void ReleaseTextureData() override;

	inline D3D12Texture* GetTexture() const { return pTexture; }
};
