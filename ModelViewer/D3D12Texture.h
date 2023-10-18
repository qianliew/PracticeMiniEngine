#pragma once
#include "D3D12TextureBuffer.h"

class D3D12Texture
{
private:
	UINT m_width;
	UINT m_height;
	UINT m_size;
	UINT64 m_bytesPerRow;
	DXGI_FORMAT m_dxgiFormat;

	std::unique_ptr<BYTE*> m_data;

public:
	UINT* GetTextureWidth();
	UINT* GetTextureHeight();
	UINT* GetTextureSize();
	UINT64* GetTextureBytesPerRow();
	BYTE* GetTextureData();

	void LoadTexture(LPCWSTR texturePath);
	void ReleaseTexture();

	std::unique_ptr<D3D12TextureBuffer> TextureBuffer;
};
