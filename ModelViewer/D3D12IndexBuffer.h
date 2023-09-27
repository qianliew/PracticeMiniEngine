#pragma once
#include "D3D12Resource.h"

class D3D12IndexBuffer : public D3D12Resource
{
private:

public:
    D3D12IndexBuffer();
    ~D3D12IndexBuffer();

    D3D12IBV* View;
};
