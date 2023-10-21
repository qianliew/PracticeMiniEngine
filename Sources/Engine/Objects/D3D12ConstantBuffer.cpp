#include "stdafx.h"
#include "D3D12ConstantBuffer.h"

D3D12ConstantBuffer::D3D12ConstantBuffer()
{
	view = new D3D12CBV();
}

D3D12ConstantBuffer::~D3D12ConstantBuffer()
{
	delete view;
}

void D3D12ConstantBuffer::CreateViewDesc()
{
	view->CBVDesc.BufferLocation = ResourceLocation->Resource->GetGPUVirtualAddress();
}

void D3D12ConstantBuffer::SetBufferSize(UINT size)
{
	view->CBVDesc.SizeInBytes = size;
}
