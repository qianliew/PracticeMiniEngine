#pragma once

class D3D12VertexBuffer : public D3D12Resource
{
private:

public:
    ~D3D12VertexBuffer();

    void CreateViewDesc() override;

    D3D12VBV* View;
};
