#pragma once

class D3D12ConstantBuffer : public D3D12Resource
{
private:
	D3D12CBV* view;
	void* startLocation;

public:
	D3D12ConstantBuffer(UINT);
	~D3D12ConstantBuffer();

	virtual void CreateViewDesc() override;
	void CopyData(void const* source, size_t size);
	void SetStartLocation(void* location);

	const D3D12CBV* GetView() { return view; }
};
