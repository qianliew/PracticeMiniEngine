#pragma once

class D3D12TextureBuffer : public D3D12Resource
{
private:
	UINT m_width;
	UINT m_height;
	UINT m_size;
	UINT64 m_bytesPerRow;
	DXGI_FORMAT m_dxgiFormat;

	std::unique_ptr<BYTE*> m_data;

public:
	D3D12SRV* View;

	~D3D12TextureBuffer();

	UINT* GetTextureWidth();
	UINT* GetTextureHeight();
	UINT* GetTextureSize();
	UINT64* GetTextureBytesPerRow();
	BYTE* GetTextureData();

	void LoadTexture(LPCWSTR texturePath);
	void CreateView() override;
	void ReleaseTexture();
};
