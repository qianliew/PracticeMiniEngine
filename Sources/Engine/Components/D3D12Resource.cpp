#include "stdafx.h"
#include "D3D12Resource.h"

D3D12Resource::D3D12Resource(const D3D12_RESOURCE_DESC& desc) :
	resourceState(D3D12_RESOURCE_STATE_GENERIC_READ),
	resourceDesc(desc)
{

}

D3D12Resource::~D3D12Resource()
{

}

void D3D12Resource::SetResourceLoaction(const ComPtr<ID3D12Resource>& resource)
{
	pResource = resource;
}
