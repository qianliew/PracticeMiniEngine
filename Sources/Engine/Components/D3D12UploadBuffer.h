#pragma once

#define UPLOAD_BUFFER_ALIGNMENT D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT

class D3D12CBV;

class D3D12UploadBuffer : public D3D12Buffer
{
private:

public:
	D3D12UploadBuffer();
	~D3D12UploadBuffer();

	virtual void CreateBuffer(
		ID3D12Device* device,
		UINT64 size,
		D3D12_RESOURCE_STATES state,
		const wchar_t* name);

	void CopyData(void const* source, UINT64 size);
	void CopyData(void const* source, UINT64 size, UINT64 offset);
	void ZeroData(UINT64 size);
};