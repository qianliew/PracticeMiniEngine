#pragma once

#define UPLOAD_BUFFER_ALIGNMENT D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT

class D3D12CBV;

class D3D12UploadBuffer : public D3D12Buffer
{
private:
	bool isConstant;
public:
	D3D12UploadBuffer(BOOL inIsConstant = FALSE);
	~D3D12UploadBuffer();

	virtual void CreateBuffer(ID3D12Device* device, UINT64 size);
	void CreateConstantBuffer(ID3D12Device* device, UINT64 size);
	void CopyData(void const* source);
	void CopyData(void const* source, size_t size);

	inline const BOOL IsConstant() const { return isConstant; }
};