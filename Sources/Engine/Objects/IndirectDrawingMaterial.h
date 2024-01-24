#pragma once
#include "LitMaterial.h"

class IndirectDrawingMaterial : public LitMaterial
{
protected:
	std::vector<std::wstring> names;
	std::vector<std::wstring> mraNames;
	std::vector<std::wstring> normalNames;

public:
	IndirectDrawingMaterial(const std::wstring inName);
	virtual ~IndirectDrawingMaterial();

	void AddTextures(const std::wstring inName);
	virtual void LoadTexture() override;
};
