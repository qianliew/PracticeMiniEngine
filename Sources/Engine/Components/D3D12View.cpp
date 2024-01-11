#include "stdafx.h"
#include "D3D12View.h"

void D3D12CBV::CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    pDevice->CreateConstantBufferView(&Desc, handle);
}

void D3D12SRV::CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    pDevice->CreateShaderResourceView(pResource, &Desc, handle);
}

void D3D12RTV::CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    pDevice->CreateRenderTargetView(pResource, &Desc, handle);
}

void D3D12DSV::CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    pDevice->CreateDepthStencilView(pResource, &Desc, handle);
}

void D3D12UAV::CreateView(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    pDevice->CreateUnorderedAccessView(pResource, nullptr, &Desc, handle);
}

void D3D12UAV::CreateViewWithCounterResource(const ComPtr<ID3D12Device>& pDevice, const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    pDevice->CreateUnorderedAccessView(pResource, pResource, &Desc, handle);
}
