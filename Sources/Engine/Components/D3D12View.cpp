#include "stdafx.h"
#include "D3D12View.h"

void D3D12CBV::CreateView(const ComPtr<ID3D12Device>& device)
{
    device->CreateConstantBufferView(&Desc, CPUHandle);
}

void D3D12SRV::CreateView(const ComPtr<ID3D12Device>& device)
{
    device->CreateShaderResourceView(resource, &Desc, CPUHandle);
}

void D3D12CBV::SetResource(ID3D12Resource* pResource)
{

}

void D3D12SRV::SetResource(ID3D12Resource* pResource)
{
    resource = pResource;
}
