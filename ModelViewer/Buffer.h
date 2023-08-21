#pragma once

class Buffer
{
protected:
	ComPtr<ID3D12Resource> m_buffer;
	void* m_startLocation;
	UINT m_bufferSize;
	UINT m_dataSize;

public:
	ComPtr<ID3D12Resource> GetBuffer();
	void* GetStartLocation();
	UINT GetDataSize();
};