#include "stdafx.h"
#include "D3D12View.h"

void D3D12CBV::CreateView(const ComPtr<ID3D12Device>& pDevice)
{
    pDevice->CreateConstantBufferView(&Desc, CPUHandle);
}

void D3D12SRV::CreateView(const ComPtr<ID3D12Device>& pDevice)
{
    pDevice->CreateShaderResourceView(pResource, &Desc, CPUHandle);
}

void D3D12DSV::CreateView(const ComPtr<ID3D12Device>& pDevice)
{
    pDevice->CreateDepthStencilView(pResource, &Desc, CPUHandle);
}
