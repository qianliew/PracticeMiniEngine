#include "stdafx.h"
#include "D3D12ConstantBuffer.h"

D3D12ConstantBuffer::D3D12ConstantBuffer(UINT size) :
	D3D12Resource(GET_CONSTANT_BUFFER_SIZE(size))
{
	view = new D3D12CBV();
}

D3D12ConstantBuffer::~D3D12ConstantBuffer()
{
	delete view;
}

void D3D12ConstantBuffer::CreateViewDesc()
{
	view->CBVDesc.BufferLocation = resourceLocation->Resource->GetGPUVirtualAddress();
	view->CBVDesc.SizeInBytes = size;
}
