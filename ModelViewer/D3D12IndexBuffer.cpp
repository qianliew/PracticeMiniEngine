#include "stdafx.h"
#include "D3D12IndexBuffer.h"

D3D12IndexBuffer::D3D12IndexBuffer()
{
	View = new D3D12IBV();
}

D3D12IndexBuffer::~D3D12IndexBuffer()
{
	delete View;
}
