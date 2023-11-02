#pragma once
#include "D3D12TextureBuffer.h"

enum class D3D12TextureType
{
	ShaderResource = 0,
	RenderTarget = 1,
	DepthStencil = 2,
};

class D3D12Texture
{
private:
	UINT id;
	UINT width;
	UINT height;
	DXGI_FORMAT dxgiFormat;

	const UINT kMipCount = 10;
	const std::wstring kMipSuffix = L"_mip";
	const std::wstring kDefaultTexturePath = L"..\\Assets\\default_mip.png";

	std::map<UINT, BYTE*> pData;
	D3D12Resource* pTextureBuffer;

	// Helper functions
	std::wstring GetTexturePath(std::wstring texturePath, UINT mipIndex);
	std::wstring GetDefaultTexturePath(UINT mipSize);

public:
	D3D12Texture(UINT inID);
	D3D12Texture(UINT inID, UINT inWidth, UINT inHeght);
	~D3D12Texture();

	inline const BYTE* GetTextureDataAt(UINT index) { return pData[index]; }
	inline D3D12Resource* GetTextureBuffer() { return pTextureBuffer; }
	inline const UINT GetTextureID() const { return id; }
	inline const UINT GetMipCount() const { return min(kMipCount, log2(width)); }

	void LoadTexture(std::wstring& texturePath);
	void CreateTexture(D3D12TextureType, BOOL hasMip = FALSE);
	void ReleaseTextureData();
	void ReleaseTextureBuffer();
	void CreateSampler();

	std::unique_ptr<D3D12Sampler> TextureSampler;
};
