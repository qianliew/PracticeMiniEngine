#pragma once

class D3D12ConstantBuffer : public D3D12Resource
{
private:
	D3D12CBV* view;

public:
	D3D12ConstantBuffer();
	~D3D12ConstantBuffer();

	void* StartLocation;

	void CreateViewDesc() override;
	void SetBufferSize(UINT);
	const D3D12CBV* GetView() { return view; }
};
