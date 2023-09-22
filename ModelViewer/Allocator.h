#pragma once
#include "Texture.h"
#include <unordered_map>

enum class UploadBufferType
{
	Constant = 0,
	Index = 1,
	Vertex = 2,
	Texture = 3,
	Count = 4,
};

class Allocator
{
private:
	ComPtr<ID3D12Device> m_device;
	UploadBuffer* UploadBufferPool[(int)UploadBufferType::Count][2];
	std::unordered_map<void*, DefaultBuffer*> DefaultBufferPool;

public:
	Allocator(ComPtr<ID3D12Device>& device);

	void AllocateUploadBuffer(UploadBuffer* &pBuffer);
	void AllocateTextureBuffer(Texture* pTexture);
};
