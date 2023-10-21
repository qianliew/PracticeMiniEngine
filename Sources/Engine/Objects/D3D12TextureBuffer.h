#pragma once

class D3D12TextureBuffer : public D3D12Resource
{
private:

public:
	D3D12SRV* View;

	~D3D12TextureBuffer();

	void CreateViewDesc() override;
};
