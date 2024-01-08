#include "stdafx.h"
#include "D3D12BufferManager.h"

D3D12BufferManager::D3D12BufferManager(ComPtr<ID3D12Device>& device) :
    pDevice(device)
{
    for (int i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++)
    {
        tempUploadBufferPool[i] = nullptr;
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
    for (auto it = uploadBufferPool.begin(); it != uploadBufferPool.end(); it++)
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

void D3D12BufferManager::AllocateTempUploadBuffer(
    D3D12UploadBuffer* pBuffer,
    UINT64 size,
    const wchar_t* name)
{
    D3D12UploadBuffer** ppBuffer = tempUploadBufferPool;

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

void D3D12BufferManager::ReleaseTempUploadBuffer()
{
    for (int i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++)
    {
        if (tempUploadBufferPool[i] != nullptr)
        {
            delete tempUploadBufferPool[i];
            tempUploadBufferPool[i] = nullptr;
        }
    }
}

void D3D12BufferManager::AllocateUploadBuffer(
    D3D12Resource* pResource,
    D3D12_RESOURCE_STATES state,
    const wchar_t* name)
{
    if (pResource->GetResource().Get() == nullptr
        || uploadBufferPool.find(pResource) == uploadBufferPool.end())
    {
        D3D12UploadBuffer* pbuffer = new D3D12UploadBuffer();
        pbuffer->CreateBuffer(pDevice.Get(), pResource->GetResourceDesc(), state, name);
        uploadBufferPool.insert(std::make_pair(pResource, pbuffer));
        pResource->SetResourceState(state);
        pResource->SetResourceLoaction(pbuffer->ResourceLocation.Resource);
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
    const UINT size = 1024;
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_NONE);
    globalConstantBuffer = new D3D12ConstantBuffer(desc, size);
    AllocateUploadBuffer(globalConstantBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, L"GlobalConstantBuffer");

    if (uploadBufferPool[globalConstantBuffer] != nullptr)
    {
        globalConstantBuffer->SetStartLocation(uploadBufferPool[globalConstantBuffer]->GetStartLocation());
    }
}

void D3D12BufferManager::AllocatePerObjectConstantBuffers(UINT offset)
{
    if (perObjectConstantBuffers[offset] != nullptr)
    {
        delete perObjectConstantBuffers[offset];
    }

    const UINT size = 1024;
    D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size, D3D12_RESOURCE_FLAG_NONE);
    perObjectConstantBuffers[offset] = new D3D12ConstantBuffer(desc, size);
    AllocateUploadBuffer(perObjectConstantBuffers[offset], D3D12_RESOURCE_STATE_GENERIC_READ, L"PerObjectConstantBuffer");
    if (uploadBufferPool[perObjectConstantBuffers[offset]])
    {
        perObjectConstantBuffers[offset]->SetStartLocation(uploadBufferPool[perObjectConstantBuffers[offset]]->GetStartLocation());
    }
}

D3D12ConstantBuffer* D3D12BufferManager::GetPerObjectConstantBufferAtIndex(UINT index)
{
    if (perObjectConstantBuffers[index] != nullptr)
    {
        return perObjectConstantBuffers[index];
    }
    return nullptr;
}
