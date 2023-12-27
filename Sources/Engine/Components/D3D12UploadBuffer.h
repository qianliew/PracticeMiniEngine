#pragma once

#define UPLOAD_BUFFER_ALIGNMENT D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT

class D3D12CBV;

class D3D12UploadBuffer : public D3D12Buffer
{
private:
	bool isConstant;
	D3D12_RESOURCE_DESC resourceDesc;

public:
	D3D12UploadBuffer(BOOL inIsConstant = FALSE);
	~D3D12UploadBuffer();

	virtual void CreateBuffer(
		ID3D12Device* device,
		UINT64 size,
		D3D12_RESOURCE_STATES state,
		const wchar_t* name);
	virtual void CreateBuffer(
		ID3D12Device* device,
		const D3D12_RESOURCE_DESC& desc,
		D3D12_RESOURCE_STATES state,
		const wchar_t* name);
	void CopyData(void const* source, UINT64 size);
	void CopyData(void const* source, UINT64 size, UINT64 offset);

	inline const BOOL IsConstant() const { return isConstant; }
	inline const D3D12_RESOURCE_DESC GetResourceDesc() const { return resourceDesc; }
};