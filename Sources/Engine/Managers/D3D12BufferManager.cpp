#include "stdafx.h"
#include "D3D12BufferManager.h"

#define BLOCK_SIZE_TYPE_1 16777216
#define BLOCK_SIZE_TYPE_2 1048576
#define BLOCK_SIZE_TYPE_3 1024
#define BLOCK_SIZE_TYPE_4 256

D3D12BufferManager::D3D12BufferManager(ComPtr<ID3D12Device>& device) :
    device(device)
{

}

D3D12BufferManager::~D3D12BufferManager()
{
    for (UINT i = 0; i < (UINT)UploadBufferType::Count; i++)
    {
        D3D12UploadBuffer** ppBuffer = UploadBufferPool[i];
        while (*ppBuffer != nullptr)
        {
            delete* ppBuffer;
            *ppBuffer = nullptr;
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

    for (UINT i = 0; *ppBuffer == nullptr && i < MAX_UPLOAD_BUFFER_COUNT; i++, ppBuffer++)
    {
        *ppBuffer = new D3D12UploadBuffer();
        (*ppBuffer)->CreateBuffer(device.Get(),
            type == UploadBufferType::Texture ? BLOCK_SIZE_TYPE_1
            : type == UploadBufferType::Vertex ? BLOCK_SIZE_TYPE_2
            : BLOCK_SIZE_TYPE_3);
        pBuffer = *ppBuffer;
        break;
    }
}

void D3D12BufferManager::AllocateDefaultBuffer(D3D12Resource* pResource)
{
    if (pResource->GetResourceLocation() == nullptr 
        || DefaultBufferPool.find(pResource->GetResourceLocation()) == DefaultBufferPool.end())
    {
        D3D12DefaultBuffer* pbuffer = new D3D12DefaultBuffer();
        pbuffer->CreateBuffer(device.Get(), pResource->GetResourceDesc());
        DefaultBufferPool.insert(std::make_pair(pResource->GetResourceLocation(), pbuffer));
        pResource->SetResourceLoaction(pbuffer->ResourceLocation);
    }
}

// Overflow case and Initialization problem.
void D3D12BufferManager::AllocateGlobalConstantBuffer()
{
    globalConstantBuffer = new D3D12ConstantBuffer(BLOCK_SIZE_TYPE_3);
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

    perObjectConstantBuffers[offset] = new D3D12ConstantBuffer(BLOCK_SIZE_TYPE_4);
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
