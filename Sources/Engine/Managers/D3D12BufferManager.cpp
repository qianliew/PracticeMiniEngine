#include "stdafx.h"
#include "D3D12BufferManager.h"

#define BLOCK_SIZE_TYPE_1 2048 * 2048 * 4
#define BLOCK_SIZE_TYPE_2 1048576
#define BLOCK_SIZE_TYPE_3 1024
#define BLOCK_SIZE_TYPE_4 256

D3D12BufferManager::D3D12BufferManager(ComPtr<ID3D12Device>& device) :
    device(device)
{
    for (UINT i = (UINT)UploadBufferType::Constant; i < (UINT)UploadBufferType::Count; i++)
    {
        D3D12UploadBuffer** ppBuffer = UploadBufferPool[i];
        while (*ppBuffer != nullptr)
        {
            *ppBuffer = nullptr;
            ppBuffer++;
        }
    }
}

D3D12BufferManager::~D3D12BufferManager()
{
    for (UINT i = (UINT)UploadBufferType::Constant; i < (UINT)UploadBufferType::Count; i++)
    {
        D3D12UploadBuffer** ppBuffer = UploadBufferPool[i];
        while (*ppBuffer != nullptr)
        {
            delete* ppBuffer;
            *ppBuffer = nullptr;
            ppBuffer++;
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

void D3D12BufferManager::AllocateUploadBuffer(D3D12UploadBuffer* &pBuffer, UploadBufferType type)
{
    D3D12UploadBuffer** ppBuffer = UploadBufferPool[(UINT)type];

    for (UINT i = 0; i < MAX_UPLOAD_BUFFER_COUNT; i++, ppBuffer++)
    {
        if (*ppBuffer != nullptr)
        {
            continue;
        }

        *ppBuffer = new D3D12UploadBuffer();
        (*ppBuffer)->CreateBuffer(device.Get(),
            type == UploadBufferType::Texture ? BLOCK_SIZE_TYPE_1
            : type == UploadBufferType::Vertex ? BLOCK_SIZE_TYPE_2
            : BLOCK_SIZE_TYPE_3);
        pBuffer = *ppBuffer;
        break;
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
        pbuffer->CreateBuffer(device.Get(), &pResource->GetResourceDesc(), state, clearValue);
        DefaultBufferPool.insert(std::make_pair(pResource, pbuffer));
        pResource->SetResourceLoaction(pbuffer->ResourceLocation);
    }
}

// Overflow case and Initialization problem.
void D3D12BufferManager::AllocateGlobalConstantBuffer()
{
    D3D12_RESOURCE_DESC desc;
    globalConstantBuffer = new D3D12ConstantBuffer(desc, BLOCK_SIZE_TYPE_3);
    D3D12UploadBuffer** ppBuffer = UploadBufferPool[(UINT)UploadBufferType::Constant];

    *ppBuffer = new D3D12UploadBuffer();
    (*ppBuffer)->CreateConstantBuffer(device.Get(), BLOCK_SIZE_TYPE_3);
    globalConstantBuffer->SetResourceLoaction((*ppBuffer)->ResourceLocation);
    globalConstantBuffer->SetStartLocation((*ppBuffer)->GetStartLocation());
}

void D3D12BufferManager::AllocatePerObjectConstantBuffers(UINT offset)
{
    if (perObjectConstantBuffers[offset] != nullptr)
    {
        delete perObjectConstantBuffers[offset];
    }

    D3D12_RESOURCE_DESC desc;
    perObjectConstantBuffers[offset] = new D3D12ConstantBuffer(desc, BLOCK_SIZE_TYPE_4);
    D3D12UploadBuffer** ppBuffer = UploadBufferPool[(UINT)UploadBufferType::Constant];

    *ppBuffer = new D3D12UploadBuffer();
    (*ppBuffer)->CreateConstantBuffer(device.Get(), BLOCK_SIZE_TYPE_4);
    perObjectConstantBuffers[offset]->SetResourceLoaction((*ppBuffer)->ResourceLocation);
    perObjectConstantBuffers[offset]->SetStartLocation((*ppBuffer)->GetStartLocation());
}

D3D12ConstantBuffer* D3D12BufferManager::GetPerObjectConstantBufferAtIndex(UINT index)
{
    if (perObjectConstantBuffers[index] != nullptr)
    {
        return perObjectConstantBuffers[index];
    }
    return nullptr;
}
