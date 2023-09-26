#include "stdafx.h"
#include "D3D12DescriptorHeapManager.h"

D3D12DescriptorHeapManager::D3D12DescriptorHeapManager(ComPtr<ID3D12Device> &device)
{
    // Describe and create a shader resource view (SRV) descriptor heap.

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

    m_srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void D3D12DescriptorHeapManager::GetSRVHandle(D3D12SRV* handle, INT offset)
{
    handle->CPUHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
    handle->CPUHandle.Offset(offset, m_srvDescriptorSize);
}

void D3D12DescriptorHeapManager::SetSRVs(ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    ID3D12DescriptorHeap* heap[] = { m_srvHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(1, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
}
