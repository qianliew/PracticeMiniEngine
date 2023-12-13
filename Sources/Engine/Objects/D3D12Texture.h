#pragma once
#include "D3D12ShaderResourceBuffer.h"

enum class D3D12TextureType
{
	ShaderResource = 0,
	RenderTarget = 1,
	DepthStencil = 2,
	UnorderedAccess = 3,
};

class D3D12Texture
{
private:
	UINT srvID;
	UINT rtvID;
	UINT width;
	UINT height;
	UINT mipLevel;
	UINT slice;
	D3D12_SRV_DIMENSION srvDimension;
	DXGI_FORMAT dxgiFormat;
	D3D12TextureType type;

	const std::wstring kMipSuffix = L"_mip";
	const std::wstring kDefaultTexturePath = L"..\\Assets\\default_mip.png";
	const std::wstring kDefaultMRATexturePath = L"..\\Assets\\default_mra_mip.png";
	const std::wstring kDefaultNormalTexturePath = L"..\\Assets\\default_n_mip.png";
	const std::wstring kCubemapPX = L"_px.png";
	const std::wstring kCubemapNX = L"_nx.png";
	const std::wstring kCubemapPY = L"_py.png";
	const std::wstring kCubemapNY = L"_ny.png";
	const std::wstring kCubemapPZ = L"_pz.png";
	const std::wstring kCubemapNZ = L"_nz.png";

	std::map<UINT, BYTE*> pData;
	D3D12Resource* pTextureBuffer;

	// Helper functions
	IWICImagingFactory* pFactory = NULL;
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pFrameDecode = NULL;
	IWICFormatConverter* pConverter = NULL;

	void LoadSingleTexture(std::wstring& texturePath, UINT index);
	std::wstring GetTexturePath(std::wstring texturePath, UINT mipIndex);
	std::wstring GetDefaultMipTexturePath(std::wstring texturePath, UINT mipSize);

public:
	D3D12Texture(UINT inSRVID, UINT inRTVID = -1);
	D3D12Texture(
		UINT inSRVID,
		UINT inRTVID,
		UINT inWidth,
		UINT inHeght,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	~D3D12Texture();

	inline const BYTE* GetTextureDataAt(UINT index) { return pData[index]; }
	inline D3D12Resource* GetTextureBuffer() { return pTextureBuffer; }
	inline const UINT GetTextureID() const { return srvID; }
	inline const UINT GetRenderTargetID() const { return rtvID; }
	inline const UINT GetSubresourceNum() const { return mipLevel * slice; }
	inline const D3D12TextureType GetType() const { return type; }

	void LoadTexture(
		std::wstring& texturePath,
		UINT mipLevel = 1,
		D3D12_SRV_DIMENSION srvDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		UINT slice = 1);
	void CreateTexture(D3D12TextureType type);
	void ReleaseTextureData();
	void ReleaseTextureBuffer();
	void CreateSampler();

	std::unique_ptr<D3D12Sampler> TextureSampler;
};
