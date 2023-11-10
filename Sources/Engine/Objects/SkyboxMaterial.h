#pragma once
#include "AbstractMaterial.h"

class SkyboxMaterial : public AbstractMaterial
{
private:

public:
	SkyboxMaterial(std::wstring inName);
	~SkyboxMaterial();

	virtual void LoadTexture() override;
	virtual void ReleaseTextureData() override;
};
