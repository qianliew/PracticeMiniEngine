#pragma once

#define UPLOAD_BUFFER_ALIGNMENT D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT

class D3D12CBV;

class D3D12UploadBuffer : public D3D12Buffer
{
public:
	D3D12UploadBuffer(const D3D12_RESOURCE_DESC& desc);
	virtual ~D3D12UploadBuffer();

	virtual void CreateBuffer(
		ID3D12Device* device,
		const wchar_t* name,
		const D3D12_CLEAR_VALUE* clearValue,
		const D3D12_RESOURCE_STATES state) override;

	void CopyData(void const* source, UINT64 size);
	void CopyData(void const* source, UINT64 size, UINT64 offset);
	void ZeroData(UINT64 size);
};