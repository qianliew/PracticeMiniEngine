#pragma once

class Texture
{
private:
	UINT m_width;
	UINT m_height;
	UINT m_size;
	UINT m_bytesPerRow;
	DXGI_FORMAT m_dxgiFormat;

	std::unique_ptr<BYTE*> m_data;

public:
	~Texture();

	UINT GetTextureWidth();
	UINT GetTextureHeight();
	UINT GetTextureSize();
	UINT GetTextureBytesPerRow();
	DXGI_FORMAT GetTextureDXGIFormat();
	BYTE* GetTextureData();

	void LoadTexture(LPCWSTR texturePath);
	void ReleaseTexture();
};