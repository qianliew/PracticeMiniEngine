#include "stdafx.h"
#include "D3D12BufferManager.h"

#define BLOCK_SIZE_TYPE_1 16777216
#define BLOCK_SIZE_TYPE_2 1048576
#define BLOCK_SIZE_TYPE_3 1024

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
void D3D12BufferManager::AllocateConstantBufferHelper(
    std::shared_ptr<D3D12ConstantBuffer>& pBuffer,
    D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    pBuffer = std::make_unique<D3D12ConstantBuffer>(BLOCK_SIZE_TYPE_3);
    D3D12UploadBuffer** ppBuffer = UploadBufferPool[(UINT)UploadBufferType::Constant];

    *ppBuffer = new D3D12UploadBuffer();
    (*ppBuffer)->CreateConstantBuffer(device.Get(), BLOCK_SIZE_TYPE_3);
    pBuffer->SetResourceLoaction((*ppBuffer)->ResourceLocation);
    pBuffer->SetStartLocation((*ppBuffer)->GetStartLocation());

    pBuffer->CreateViewDesc();
    device->CreateConstantBufferView(&pBuffer->GetView()->CBVDesc, handle);
}

void D3D12BufferManager::AllocateGlobalConstantBuffer(D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    AllocateConstantBufferHelper(globalConstantBuffer, handle);
}

void D3D12BufferManager::AllocatePerObjectConstantBuffer(D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    AllocateConstantBufferHelper(perObjectConstantBuffer, handle);
}
