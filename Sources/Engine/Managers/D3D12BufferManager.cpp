#include "stdafx.h"
#include "D3D12BufferManager.h"

D3D12BufferManager::D3D12BufferManager(ComPtr<ID3D12Device>& device) :
    pDevice(device)
{
    for (int i = 0; i < MAX_TEMP_UPLOAD_BUFFER_COUNT; i++)
    {
        tempUploadBufferPool[i] = nullptr;
    }
    for (int i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++)
    {
        uploadBufferPool[i] = nullptr;
    }
    for (int i = 0; i < MAX_READBACK_BUFFER_COUNT; i++)
    {
        readbackBufferPool[i] = nullptr;
    }
}

D3D12BufferManager::~D3D12BufferManager()
{
    // Release buffers in pools.
    ReleaseTempUploadBuffer();
    for (int i = 0; i < MAX_READBACK_BUFFER_COUNT; i++)
    {
        if (readbackBufferPool[i] != nullptr)
        {
            delete readbackBufferPool[i];
            readbackBufferPool[i] = nullptr;
        }
    }
    for (int i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++)
    {
        if (uploadBufferPool[i] != nullptr)
        {
            delete uploadBufferPool[i];
            uploadBufferPool[i] = nullptr;
        }
    }
    for (auto it = defaultBufferPool.begin(); it != defaultBufferPool.end(); it++)
    {
        if (it->second != nullptr)
        {
            delete it->second;
            it->second = nullptr;
        }
    }

    delete globalConstantBuffer;
    for (auto it = perObjectConstantBuffers.begin(); it != perObjectConstantBuffers.end(); it++)
    {
        if (it->second != nullptr)
        {
            delete it->second;
            it->second = nullptr;
        }
    }
}

void D3D12BufferManager::AllocateTempUploadBuffer(
    D3D12UploadBuffer* pBuffer,
    const D3D12_RESOURCE_STATES state,
    const wchar_t* name)
{
    D3D12UploadBuffer** ppBuffer = tempUploadBufferPool;

    for (UINT i = 0; i < MAX_TEMP_UPLOAD_BUFFER_COUNT; i++, ppBuffer++)
    {
        if (*ppBuffer != nullptr)
        {
            continue;
        }

        *ppBuffer = pBuffer;
        pBuffer->CreateBuffer(pDevice.Get(), name, nullptr, state);
        break;
    }
}

void D3D12BufferManager::ReleaseTempUploadBuffer()
{
    for (int i = 0; i < MAX_TEMP_UPLOAD_BUFFER_COUNT; i++)
    {
        if (tempUploadBufferPool[i] != nullptr)
        {
            delete tempUploadBufferPool[i];
            tempUploadBufferPool[i] = nullptr;
        }
    }
}

void D3D12BufferManager::AllocateUploadBuffer(
    D3D12UploadBuffer* pBuffer,
    const D3D12_RESOURCE_STATES state,
    const wchar_t* name)
{
    D3D12UploadBuffer** ppBuffer = uploadBufferPool;

    for (UINT i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++, ppBuffer++)
    {
        if (*ppBuffer != nullptr)
        {
            continue;
        }

        *ppBuffer = pBuffer;
        pBuffer->CreateBuffer(pDevice.Get(), name, nullptr, state);
        break;
    }
}

void D3D12BufferManager::AllocateReadbackBuffer(
    D3D12ReadbackBuffer* pBuffer,
    const D3D12_RESOURCE_STATES state,
    const wchar_t* name)
{
    D3D12ReadbackBuffer** ppBuffer = readbackBufferPool;

    for (UINT i = 0; i < MAX_READBACK_BUFFER_COUNT; i++, ppBuffer++)
    {
        if (*ppBuffer != nullptr)
        {
            continue;
        }

        *ppBuffer = pBuffer;
        pBuffer->CreateBuffer(pDevice.Get(), name, nullptr, state);
        break;
    }
}

void D3D12BufferManager::AllocateDefaultBuffer(
    D3D12Resource* pResource,
    const D3D12_RESOURCE_DESC& desc,
    const D3D12_RESOURCE_STATES state,
    const wchar_t* name,
    const D3D12_CLEAR_VALUE* clearValue)
{
    if (pResource->GetResource().Get() == nullptr
        || defaultBufferPool.find(pResource) == defaultBufferPool.end())
    {
        D3D12DefaultBuffer* pbuffer = new D3D12DefaultBuffer(desc);
        pbuffer->CreateBuffer(pDevice.Get(), name, clearValue, state);
        pResource->SetBuffer(pbuffer);
        defaultBufferPool.insert(std::make_pair(pResource, pbuffer));
    }
}

// Overflow case and Initialization problem.
void D3D12BufferManager::AllocateGlobalConstantBuffer()
{
    const UINT64 size = 1024;
    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
    D3D12UploadBuffer* uploadBuffer = new D3D12UploadBuffer(resourceDesc);
    AllocateUploadBuffer(uploadBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, L"GlobalConstantBuffer");

    globalConstantBuffer = new D3D12ConstantBuffer(desc, size);
    globalConstantBuffer->SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
    globalConstantBuffer->SetResourceLoaction(uploadBuffer->GetResource());
    globalConstantBuffer->SetStartLocation(uploadBuffer->GetLocation());
}

void D3D12BufferManager::AllocatePerObjectConstantBuffers(UINT offset)
{
    if (perObjectConstantBuffers[offset] != nullptr)
    {
        delete perObjectConstantBuffers[offset];
    }

    const UINT64 size = 1024;
    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_NONE);
    D3D12UploadBuffer* uploadBuffer = new D3D12UploadBuffer(resourceDesc);
    AllocateUploadBuffer(uploadBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, L"PerObjectConstantBuffer");

    perObjectConstantBuffers[offset] = new D3D12ConstantBuffer(desc, size);
    perObjectConstantBuffers[offset]->SetResourceState(D3D12_RESOURCE_STATE_GENERIC_READ);
    perObjectConstantBuffers[offset]->SetResourceLoaction(uploadBuffer->GetResource());
    perObjectConstantBuffers[offset]->SetStartLocation(uploadBuffer->GetLocation());
}

D3D12ConstantBuffer* D3D12BufferManager::GetPerObjectConstantBufferAtIndex(UINT index)
{
    if (perObjectConstantBuffers[index] != nullptr)
    {
        return perObjectConstantBuffers[index];
    }
    return nullptr;
}
