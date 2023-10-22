#include "stdafx.h"
#include "D3D12Resource.h"

D3D12Resource::D3D12Resource(UINT size) :
	size(size)
{
	resourceLocation = new D3D12ResourceLocation();
}

D3D12Resource::~D3D12Resource()
{
	delete resourceLocation;
}

void  D3D12Resource::SetResourceLoaction(const D3D12ResourceLocation* location)
{
	resourceLocation->Resource = location->Resource;
}

void D3D12Resource::SetResourceDesc(D3D12_RESOURCE_DESC* desc)
{
	resourceDesc = desc;
}
