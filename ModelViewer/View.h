#pragma once
#include "stdafx.h"

class View
{
};

class SRV : public View
{
public:
	CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
};

class VBV : public View
{
public:
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
};