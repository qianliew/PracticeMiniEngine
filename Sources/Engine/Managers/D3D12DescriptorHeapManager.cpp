#include "stdafx.h"
#include "D3D12DescriptorHeapManager.h"

D3D12DescriptorHeapManager::D3D12DescriptorHeapManager(ComPtr<ID3D12Device> &device)
{
    // Describe and create a shader resource view (SRV) descriptor heap.

    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap)));

    srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 1;
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerHeap)));

    samplerDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void D3D12DescriptorHeapManager::GetSRVHandle(D3D12SRV* handle, INT offset)
{
    handle->CPUHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
    handle->CPUHandle.Offset(offset, srvDescriptorSize);
}

void D3D12DescriptorHeapManager::GetSamplerHandle(D3D12Sampler* const sampler, INT offset)
{
    sampler->CPUHandle = samplerHeap->GetCPUDescriptorHandleForHeapStart();
    sampler->CPUHandle.Offset(offset, samplerDescriptorSize);
}

void D3D12DescriptorHeapManager::SetSRVs(ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    ID3D12DescriptorHeap* heap[] = { srvHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());
}

void D3D12DescriptorHeapManager::SetSamplers(ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    ID3D12DescriptorHeap* heap[] = { samplerHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(2, samplerHeap->GetGPUDescriptorHandleForHeapStart());
}
