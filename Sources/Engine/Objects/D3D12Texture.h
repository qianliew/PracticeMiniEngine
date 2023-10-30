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
	UINT width;
	UINT height;
	UINT64 size;
	UINT64 bytesPerRow;
	DXGI_FORMAT dxgiFormat;

	BYTE* pData;

public:
	D3D12Texture();
	D3D12Texture(UINT inWidth, UINT inHeght);
	~D3D12Texture();

	inline const BYTE* GetTextureData() { return pData; }

	void LoadTexture(LPCWSTR texturePath);
	void CreateTexture(D3D12TextureType);
	void ReleaseTexture();
	void CreateSampler();

	D3D12Resource* TextureBuffer;
	std::unique_ptr<D3D12Sampler> TextureSampler;
};
