#pragma once

class D3D12IndexBuffer : public D3D12Resource
{
private:

public:
    ~D3D12IndexBuffer();

    void CreateView() override;

    D3D12IBV* View;
};
