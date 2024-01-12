#include "stdafx.h"
#include "D3D12Resource.h"

D3D12Resource::D3D12Resource() :
	pBuffer(nullptr)
{

}

D3D12Resource::~D3D12Resource()
{

}

void D3D12Resource::SetBuffer(D3D12Buffer* buffer)
{
	pBuffer = buffer;
}
