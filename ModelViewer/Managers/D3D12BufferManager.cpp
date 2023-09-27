#include "stdafx.h"
#include "D3D12BufferManager.h"

#define BLOCK_SIZE_TYPE_1 16777216
#define BLOCK_SIZE_TYPE_2 1048576
#define BLOCK_SIZE_TYPE_3 1024

D3D12BufferManager::D3D12BufferManager(ComPtr<ID3D12Device>& device)
{
	m_device = device;
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
        (**ppBuffer).CreateBuffer(m_device.Get(),
            type == UploadBufferType::Texture ? BLOCK_SIZE_TYPE_1
            : type == UploadBufferType::Vertex ? BLOCK_SIZE_TYPE_2
            : BLOCK_SIZE_TYPE_3);
        pBuffer = *ppBuffer;
        break;
    }
}

void D3D12BufferManager::AllocateDefaultBuffer(D3D12ResourceLocation* pResourceLocation, D3D12_RESOURCE_DESC* pDESC)
{
    if (DefaultBufferPool.find(pResourceLocation) == DefaultBufferPool.end())
    {
        D3D12DefaultBuffer* pbuffer = new D3D12DefaultBuffer();
        pbuffer->CreateBuffer(m_device.Get(), pDESC);
        DefaultBufferPool.insert(std::make_pair(pResourceLocation, pbuffer));
        pResourceLocation->Resource = pbuffer->ResourceLocation->Resource;
    }
}
