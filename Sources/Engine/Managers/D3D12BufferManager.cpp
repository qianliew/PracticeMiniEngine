#include "stdafx.h"
#include "D3D12BufferManager.h"

D3D12BufferManager::D3D12BufferManager(ComPtr<ID3D12Device>& device) :
    pDevice(device)
{
    for (int i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++)
    {
        UploadBufferPool[i] = nullptr;
    }
}

D3D12BufferManager::~D3D12BufferManager()
{
    for (int i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++)
    {
        if (UploadBufferPool[i] != nullptr)
        {
            delete UploadBufferPool[i];
            UploadBufferPool[i] = nullptr;
        }
    }

    for (auto it = DefaultBufferPool.begin(); it != DefaultBufferPool.end(); it++)
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

void D3D12BufferManager::AllocateUploadBuffer(D3D12UploadBuffer* pBuffer, UINT64 size)
{
    D3D12UploadBuffer** ppBuffer = UploadBufferPool;

    for (UINT i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++, ppBuffer++)
    {
        if (*ppBuffer != nullptr)
        {
            continue;
        }

        *ppBuffer = pBuffer;
        pBuffer->CreateBuffer(pDevice.Get(), size);
        break;
    }
}

void D3D12BufferManager::ReleaseUploadBuffer()
{
    for (int i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++)
    {
        if (UploadBufferPool[i] != nullptr
            && !UploadBufferPool[i]->IsConstant())
        {
            delete UploadBufferPool[i];
            UploadBufferPool[i] = nullptr;
        }
    }
}

void D3D12BufferManager::AllocateDefaultBuffer(
    D3D12Resource* pResource,
    D3D12_RESOURCE_STATES state,
    const D3D12_CLEAR_VALUE* clearValue)
{
    if (pResource->GetResource() == nullptr 
        || DefaultBufferPool.find(pResource) == DefaultBufferPool.end())
    {
        D3D12DefaultBuffer* pbuffer = new D3D12DefaultBuffer();
        pbuffer->CreateBuffer(pDevice.Get(), &pResource->GetResourceDesc(), state, clearValue);
        DefaultBufferPool.insert(std::make_pair(pResource, pbuffer));
        pResource->SetResourceLoaction(pbuffer->ResourceLocation);
    }
}

// Overflow case and Initialization problem.
void D3D12BufferManager::AllocateGlobalConstantBuffer()
{
    D3D12_RESOURCE_DESC desc;
    globalConstantBuffer = new D3D12ConstantBuffer(desc, 1024);
    D3D12UploadBuffer* pBuffer = new D3D12UploadBuffer(TRUE);
    AllocateUploadBuffer(pBuffer, 1024);

    globalConstantBuffer->SetResourceLoaction(pBuffer->ResourceLocation);
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

    perObjectConstantBuffers[offset]->SetResourceLoaction(pBuffer->ResourceLocation);
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
