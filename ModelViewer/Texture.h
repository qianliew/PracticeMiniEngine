#pragma once

class Texture
{
private:
	UINT m_width;
	UINT m_height;
	UINT m_size;
	UINT m_bytesPerRow;
	DXGI_FORMAT m_dxgiFormat;

	D3D12_RESOURCE_DESC* m_desc;
	std::unique_ptr<BYTE*> m_data;

public:
	DefaultBuffer* Buffer;

	~Texture();

	UINT GetTextureWidth();
	UINT GetTextureHeight();
	UINT GetTextureSize();
	UINT GetTextureBytesPerRow();
	BYTE* GetTextureData();
	D3D12_RESOURCE_DESC* GetTextureDesc();

	void LoadTexture(LPCWSTR texturePath);
	void ReleaseTexture();
};
