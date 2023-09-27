#pragma once
#include "D3D12Resource.h"

class D3D12VertexBuffer : public D3D12Resource
{
private:

public:
    D3D12VertexBuffer();
    ~D3D12VertexBuffer();

    D3D12VBV* View;
};
