#include "stdafx.h"
#include "Allocator.h"

#define BLOCK_SIZE_TYPE_1 16777216
#define BLOCK_SIZE_TYPE_2 1048576

Allocator::Allocator(ComPtr<ID3D12Device>& device)
{
	m_device = device;
}

void Allocator::AllocateUploadBuffer(UploadBuffer* &pBuffer, UploadBufferType type)
{
    UploadBuffer** ppBuffer = UploadBufferPool[(UINT)type];

    for (UINT count = 0; *ppBuffer == nullptr && count < (UINT)UploadBufferType::Count; count++, ppBuffer++)
    {
        *ppBuffer = new UploadBuffer();
        (**ppBuffer).CreateBuffer(m_device.Get(), type == UploadBufferType::Texture ? BLOCK_SIZE_TYPE_1 : BLOCK_SIZE_TYPE_2);
        pBuffer = *ppBuffer;
        break;
    }
}

void Allocator::AllocateTextureBuffer(Texture* pTexture)
{
    if (DefaultBufferPool.find(pTexture) == DefaultBufferPool.end())
    {
        DefaultBuffer* buffer = new DefaultBuffer();
        buffer->CreateBuffer(m_device.Get(), pTexture->GetTextureDesc());
        DefaultBufferPool.insert(std::make_pair(pTexture, buffer));
        pTexture->Buffer = buffer;
    }
}

void Allocator::AllocateVertexBuffer(Mesh* pMesh)
{
    if (DefaultBufferPool.find(pMesh) == DefaultBufferPool.end())
    {
        DefaultBuffer* buffer = new DefaultBuffer();
        buffer->CreateBuffer(m_device.Get(), pMesh->GetVertexDesc());
        DefaultBufferPool.insert(std::make_pair(pMesh, buffer));
        pMesh->VertexBuffer = buffer;
    }
}
