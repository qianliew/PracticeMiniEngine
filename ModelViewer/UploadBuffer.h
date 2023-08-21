#pragma once
#include "Buffer.h"

#define UPLOAD_BUFFER_ALIGNMENT D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT

class UploadBuffer : public Buffer
{
public:
	~UploadBuffer();

	ComPtr<ID3D12Resource> GetBuffer();

	void* GetStartLocation();

	UINT GetDataSize();

	void CreateBuffer(ID3D12Device* device, UINT size) override;

	void CopyData(void const* source);
};