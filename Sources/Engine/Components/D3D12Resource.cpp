#include "stdafx.h"
#include "D3D12Resource.h"

D3D12Resource::D3D12Resource(const D3D12_RESOURCE_DESC& desc) :
	resourceDesc(desc)
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
