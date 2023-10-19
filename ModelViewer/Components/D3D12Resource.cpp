#include "stdafx.h"
#include "D3D12Resource.h"

void  D3D12Resource::SetResourceLoaction(const D3D12ResourceLocation* location)
{
	ResourceLocation = new D3D12ResourceLocation();
	ResourceLocation->Resource = location->Resource;
}

void D3D12Resource::SetResourceDesc(D3D12_RESOURCE_DESC* desc)
{
	ResourceDesc = desc;
}
