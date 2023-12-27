#include "stdafx.h"
#include "D3D12DescriptorHeapManager.h"

D3D12DescriptorHeapManager::D3D12DescriptorHeapManager(ComPtr<ID3D12Device> &device, BOOL isDXR)
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
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&heapTable[SHADER_RESOURCE_VIEW_PEROBJECT])));

    srvHeapDesc.NumDescriptors = 10;
    ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&heapTable[SHADER_RESOURCE_VIEW_GLOBAL])));

    sizeTable[SHADER_RESOURCE_VIEW_PEROBJECT] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    sizeTable[SHADER_RESOURCE_VIEW_GLOBAL] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create a unordered access view (UAV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
    uavHeapDesc.NumDescriptors = 2;
    uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&heapTable[UNORDERED_ACCESS_VIEW])));

    sizeTable[UNORDERED_ACCESS_VIEW] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create a sampler descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
    samplerHeapDesc.NumDescriptors = 10;
    samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&heapTable[SAMPLER])));

    sizeTable[SAMPLER] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FRAME_COUNT + 10;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&heapTable[RENDER_TARGET_VIEW])));

    sizeTable[RENDER_TARGET_VIEW] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Describe and create a depth stencil view (DSV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&heapTable[DEPTH_STENCIL_VIEW])));

    sizeTable[DEPTH_STENCIL_VIEW] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
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

void D3D12DescriptorHeapManager::SetViews(
    ComPtr<ID3D12GraphicsCommandList>& commandList,
    UINT index,
    UINT rootIndex,
    INT offset)
{
    ID3D12DescriptorHeap* heap[] = { heapTable[index].Get() };
    
    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(heapTable[index]->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(offset, sizeTable[index]);

    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetGraphicsRootDescriptorTable(rootIndex, handle);
}

void D3D12DescriptorHeapManager::SetComputeViews(
    ComPtr<ID3D12GraphicsCommandList>& commandList,
    UINT index,
    UINT rootIndex,
    INT offset)
{
    ID3D12DescriptorHeap* heap[] = { heapTable[index].Get() };

    CD3DX12_GPU_DESCRIPTOR_HANDLE handle(heapTable[index]->GetGPUDescriptorHandleForHeapStart());
    handle.Offset(offset, sizeTable[index]);

    commandList->SetDescriptorHeaps(_countof(heap), heap);
    commandList->SetComputeRootDescriptorTable(rootIndex, handle);
}
