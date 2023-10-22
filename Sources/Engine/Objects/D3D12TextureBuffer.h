#pragma once

class D3D12TextureBuffer : public D3D12Resource
{
private:

public:
	D3D12TextureBuffer(UINT);
	~D3D12TextureBuffer();

	D3D12SRV* View;

	void CreateViewDesc() override;
};
