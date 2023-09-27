#pragma once
#include "stdafx.h"

class D3D12View
{
};

class D3D12SRV : public D3D12View
{
public:
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
};

class D3D12VBV : public D3D12View
{
public:
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
};

class D3D12IBV : public D3D12View
{
public:
	D3D12_INDEX_BUFFER_VIEW IndexBufferView;
};
