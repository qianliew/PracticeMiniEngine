#include "stdafx.h"
#include "D3D12Resource.h"

D3D12Resource::D3D12Resource()
{
	ResourceLocation = new D3D12ResourceLocation();
	ResourceDesc = new D3D12_RESOURCE_DESC();
}

D3D12Resource::~D3D12Resource()
{
	delete ResourceDesc;
}

