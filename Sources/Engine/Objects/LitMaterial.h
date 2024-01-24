#pragma once
#include "AbstractMaterial.h"

class LitMaterial : public AbstractMaterial
{
protected:
	const std::wstring kMRASuffix = L"_mra";
	const std::wstring kNormalSuffix = L"_n";

	D3D12Texture* pMRATexture;
	D3D12Texture* pNormalTexture;

	//Helper functions
	std::wstring GetMRATexturePath(std::wstring texturePath);
	std::wstring GetNormalTexturePath(std::wstring texturePath);

public:
	LitMaterial(const std::wstring inName);
	virtual ~LitMaterial();

	virtual void LoadTexture() override;
	virtual void ReleaseTextureData() override;

	inline D3D12Texture* GetMRATexture() const { return pMRATexture; }
	inline D3D12Texture* GetNormalTexture() const { return pNormalTexture; }
};
