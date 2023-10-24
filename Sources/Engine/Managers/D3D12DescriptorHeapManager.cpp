#include "stdafx.h"
#include "D3D12DescriptorHeapManager.h"

D3D12DescriptorHeapManager::D3D12DescriptorHeapManager(ComPtr<ID3D12Device> &device)
{
    // Describe and create constant buffer view (CBV) descriptor heaps.
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&heaps[CONSTANT_BUFFER_VIEW_GLOBAL])));

    cbvHeapDesc.NumDescriptors = 10;
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&heaps[CONSTANT_BUFFER_VIEW_PEROBJECT])));

    cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create a shader resource view (SRV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap)));

    srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create a sampler descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 1;
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&samplerHeap)));

    samplerDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

D3D12DescriptorHeapManager::~D3D12DescriptorHeapManager()
{

}

void D3D12DescriptorHeapManager::GetCBVHandle(D3D12CBV* view, UINT index, INT offset)
{
    view->CPUHandle = heaps[index]->GetCPUDescriptorHandleForHeapStart();
    view->CPUHandle.Offset(offset, cbvDescriptorSize);
}

void D3D12DescriptorHeapManager::GetSRVHandle(D3D12SRV* view, INT offset)
{
    view->CPUHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
    view->CPUHandle.Offset(offset, srvDescriptorSize);
}

void D3D12DescriptorHeapManager::GetSamplerHandle(D3D12Sampler* const sampler, INT offset)
{
    sampler->CPUHandle = samplerHeap->GetCPUDescriptorHandleForHeapStart();
    sampler->CPUHandle.Offset(offset, samplerDescriptorSize);
}

void D3D12DescriptorHeapManager::SetCBVs(ComPtr<ID3D12GraphicsCommandList>& commandList, UINT index, INT offset)
{
    ID3D12DescriptorHeap* heap[] = { heaps[index].Get() };
    
    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(heaps[index]->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(offset, cbvDescriptorSize);

    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(index, handle);
}

void D3D12DescriptorHeapManager::SetSRVs(ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    ID3D12DescriptorHeap* heap[] = { srvHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(SHADER_RESOURCE_VIEW, srvHeap->GetGPUDescriptorHandleForHeapStart());
}

void D3D12DescriptorHeapManager::SetSamplers(ComPtr<ID3D12GraphicsCommandList>& commandList)
{
    ID3D12DescriptorHeap* heap[] = { samplerHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(SAMPLER, samplerHeap->GetGPUDescriptorHandleForHeapStart());
}
