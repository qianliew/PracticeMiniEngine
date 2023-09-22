#include "stdafx.h"
#include "Allocator.h"

Allocator::Allocator(ComPtr<ID3D12Device>& device)
{
	m_device = device;
}

void Allocator::AllocateUploadBuffer(UploadBuffer* &pBuffer)
{
    UploadBuffer** ppBuffer = UploadBufferPool[(UINT)UploadBufferType::Texture];

    for (UINT count = 0; *ppBuffer == nullptr && count < (UINT)UploadBufferType::Count; count++, ppBuffer++)
    {
        *ppBuffer = new UploadBuffer();
        (**ppBuffer).CreateBuffer(m_device.Get(), BIG_BLOCK_SIZE);
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
