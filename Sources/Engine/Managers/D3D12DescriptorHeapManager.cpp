#include "stdafx.h"
#include "D3D12DescriptorHeapManager.h"

D3D12DescriptorHeapManager::D3D12DescriptorHeapManager(ComPtr<ID3D12Device> &device)
{
    // Describe and create constant buffer view (CBV) descriptor heaps.
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&heapTable[CONSTANT_BUFFER_VIEW_GLOBAL])));

    cbvHeapDesc.NumDescriptors = 10;
    ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&heapTable[CONSTANT_BUFFER_VIEW_PEROBJECT])));

    sizeTable[CONSTANT_BUFFER_VIEW_GLOBAL] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    sizeTable[CONSTANT_BUFFER_VIEW_PEROBJECT] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create a shader resource view (SRV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 10;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&heapTable[SHADER_RESOURCE_VIEW])));

    sizeTable[SHADER_RESOURCE_VIEW] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create a sampler descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 2;
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&heapTable[SAMPLER])));

    sizeTable[SAMPLER] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FRAME_COUNT + 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Describe and create a depth stencil view (DSV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));

    dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

D3D12DescriptorHeapManager::~D3D12DescriptorHeapManager()
{

}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapManager::GetHandle(UINT index, INT offset)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(heapTable[index]->GetCPUDescriptorHandleForHeapStart());
    handle.Offset(offset, sizeTable[index]);

    return handle;
}

void D3D12DescriptorHeapManager::GetSamplerHandle(D3D12Sampler* const sampler, INT offset)
{
    sampler->CPUHandle = heapTable[SAMPLER]->GetCPUDescriptorHandleForHeapStart();
    sampler->CPUHandle.Offset(offset, sizeTable[SAMPLER]);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapManager::GetRTVHandle(INT offset)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
    handle.Offset(offset, rtvDescriptorSize);

    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapManager::GetDSVHandle(INT offset)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE handle(dsvHeap->GetCPUDescriptorHandleForHeapStart());
    handle.Offset(offset, dsvDescriptorSize);

    return handle;
}

void D3D12DescriptorHeapManager::SetCBVs(ComPtr<ID3D12GraphicsCommandList>& commandList, UINT index, INT offset)
{
    ID3D12DescriptorHeap* heap[] = { heapTable[index].Get() };
    
    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(heapTable[index]->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(offset, sizeTable[index]);

    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(index, handle);
}

void D3D12DescriptorHeapManager::SetSRVs(ComPtr<ID3D12GraphicsCommandList>& commandList, INT offset)
{
    ID3D12DescriptorHeap* heap[] = { heapTable[SHADER_RESOURCE_VIEW].Get() };

    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(heapTable[SHADER_RESOURCE_VIEW]->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(offset, sizeTable[SHADER_RESOURCE_VIEW]);

    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(SHADER_RESOURCE_VIEW, handle);
}

void D3D12DescriptorHeapManager::SetSamplers(ComPtr<ID3D12GraphicsCommandList>& commandList, INT offset)
{
    ID3D12DescriptorHeap* heap[] = { heapTable[SAMPLER].Get() };

    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(heapTable[SAMPLER]->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(offset, sizeTable[SAMPLER]);

    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(SAMPLER, handle);
}
