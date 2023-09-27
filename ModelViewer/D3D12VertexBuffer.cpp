#include "stdafx.h"
#include "D3D12VertexBuffer.h"

D3D12VertexBuffer::D3D12VertexBuffer()
{
	View = new D3D12VBV();
}

D3D12VertexBuffer::~D3D12VertexBuffer()
{
	delete View;
}
