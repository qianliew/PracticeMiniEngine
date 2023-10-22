#pragma once

class D3D12IndexBuffer : public D3D12Resource
{
private:

public:
    D3D12IndexBuffer(UINT);
    ~D3D12IndexBuffer();

    void CreateViewDesc() override;

    D3D12IBV* View;
};
