#pragma once
#include "Buffer.h"

class DefaultBuffer : public Buffer
{
public:
	void CreateBuffer(ID3D12Device* device, const D3D12_RESOURCE_DESC* desc);
};