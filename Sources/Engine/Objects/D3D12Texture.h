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

	const UINT mipCount = 3;

	std::map<UINT, BYTE*> pData;
	D3D12Resource* pTextureBuffer;

	std::wstring GetMipPath(std::wstring texturePath, UINT index);

public:
	D3D12Texture(UINT inID);
	D3D12Texture(UINT inID, UINT inWidth, UINT inHeght);
	~D3D12Texture();

	inline const BYTE* GetTextureDataAt(UINT index) { return pData[index]; }
	inline D3D12Resource* GetTextureBuffer() { return pTextureBuffer; }
	inline const UINT GetTextureID() const { return id; }
	inline const UINT GetMipCount() const { return mipCount; }

	void LoadTexture(std::wstring& texturePath);
	void CreateTexture(D3D12TextureType, BOOL hasMip = FALSE);
	void ReleaseTexture();
	void CreateSampler();

	std::unique_ptr<D3D12Sampler> TextureSampler;
};
