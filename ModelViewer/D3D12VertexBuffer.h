#pragma once

class D3D12VertexBuffer : public D3D12Resource
{
private:

public:
    ~D3D12VertexBuffer();

    void CreateView() override;

    D3D12VBV* View;
};
