#include "stdafx.h"
#include "D3D12BufferManager.h"

D3D12BufferManager::D3D12BufferManager(ComPtr<ID3D12Device>& device) :
    pDevice(device)
{
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
    for (int i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++)
    {
        if (uploadBufferPool[i] != nullptr)
        {
            delete uploadBufferPool[i];
            uploadBufferPool[i] = nullptr;
        }
    }
    for (int i = 0; i < MAX_READBACK_BUFFER_COUNT; i++)
    {
        if (readbackBufferPool[i] != nullptr)
        {
            delete readbackBufferPool[i];
            readbackBufferPool[i] = nullptr;
        }
    }
    for (auto it = uploadBufferPool2.begin(); it != uploadBufferPool2.end(); it++)
    {
        if (it->second != nullptr)
        {
            delete it->second;
            it->second = nullptr;
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

void D3D12BufferManager::AllocateUploadBuffer(
    D3D12UploadBuffer* pBuffer,
    UINT64 size,
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
        pBuffer->CreateBuffer(pDevice.Get(), size, D3D12_RESOURCE_STATE_GENERIC_READ, name);
        break;
    }
}

void D3D12BufferManager::AllocateUploadBuffer(
    D3D12Resource* pResource,
    D3D12_RESOURCE_STATES state,
    const wchar_t* name)
{
    if (pResource->GetResource().Get() == nullptr
        || uploadBufferPool2.find(pResource) == uploadBufferPool2.end())
    {
        D3D12UploadBuffer* pbuffer = new D3D12UploadBuffer();
        pbuffer->CreateBuffer(pDevice.Get(), pResource->GetResourceDesc(), state, name);
        uploadBufferPool2.insert(std::make_pair(pResource, pbuffer));
        pResource->SetResourceState(state);
        pResource->SetResourceLoaction(pbuffer->ResourceLocation.Resource);
    }
}

void D3D12BufferManager::ReleaseUploadBuffer()
{
    for (int i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++)
    {
        if (uploadBufferPool[i] != nullptr
            && !uploadBufferPool[i]->IsConstant())
        {
            delete uploadBufferPool[i];
            uploadBufferPool[i] = nullptr;
        }
    }
}

void D3D12BufferManager::AllocateReadbackBuffer(
    D3D12ReadbackBuffer* pBuffer,
    UINT64 size,
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
        pBuffer->CreateBuffer(pDevice.Get(), size, D3D12_RESOURCE_STATE_COPY_DEST, name);
        break;
    }
}

void D3D12BufferManager::AllocateDefaultBuffer(
    D3D12Resource* pResource,
    D3D12_RESOURCE_STATES state,
    const wchar_t* name,
    const D3D12_CLEAR_VALUE* clearValue)
{
    if (pResource->GetResource().Get() == nullptr
        || defaultBufferPool.find(pResource) == defaultBufferPool.end())
    {
        D3D12DefaultBuffer* pbuffer = new D3D12DefaultBuffer();
        pbuffer->CreateBuffer(pDevice.Get(), &pResource->GetResourceDesc(), state, name, clearValue);
        defaultBufferPool.insert(std::make_pair(pResource, pbuffer));
        pResource->SetResourceState(state);
        pResource->SetResourceLoaction(pbuffer->ResourceLocation.Resource);
    }
}

// Overflow case and Initialization problem.
void D3D12BufferManager::AllocateGlobalConstantBuffer()
{
    D3D12_RESOURCE_DESC desc;
    globalConstantBuffer = new D3D12ConstantBuffer(desc, 1024);
    D3D12UploadBuffer* pBuffer = new D3D12UploadBuffer(TRUE);
    AllocateUploadBuffer(pBuffer, 1024);

    globalConstantBuffer->SetResourceLoaction(pBuffer->ResourceLocation.Resource);
    globalConstantBuffer->SetStartLocation(pBuffer->GetStartLocation());
}

void D3D12BufferManager::AllocatePerObjectConstantBuffers(UINT offset)
{
    if (perObjectConstantBuffers[offset] != nullptr)
    {
        delete perObjectConstantBuffers[offset];
    }

    D3D12_RESOURCE_DESC desc;
    perObjectConstantBuffers[offset] = new D3D12ConstantBuffer(desc, 1024);
    D3D12UploadBuffer* pBuffer = new D3D12UploadBuffer(TRUE);
    AllocateUploadBuffer(pBuffer, 1024);

    perObjectConstantBuffers[offset]->SetResourceLoaction(pBuffer->ResourceLocation.Resource);
    perObjectConstantBuffers[offset]->SetStartLocation(pBuffer->GetStartLocation());
}

D3D12ConstantBuffer* D3D12BufferManager::GetPerObjectConstantBufferAtIndex(UINT index)
{
    if (perObjectConstantBuffers[index] != nullptr)
    {
        return perObjectConstantBuffers[index];
    }
    return nullptr;
}

D3D12UploadBuffer* D3D12BufferManager::GetUploadBufferFromPool(D3D12Resource* pResource)
{
    if (uploadBufferPool2.find(pResource) != uploadBufferPool2.end())
    {
        return uploadBufferPool2[pResource];
    }
    return nullptr;
}
