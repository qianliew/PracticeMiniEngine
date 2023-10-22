#pragma once

class D3D12ConstantBuffer : public D3D12Resource
{
private:
	D3D12CBV* view;

public:
	D3D12ConstantBuffer(UINT);
	~D3D12ConstantBuffer();

	virtual void CreateViewDesc() override;

	void* StartLocation;
	const D3D12CBV* GetView() { return view; }
};
