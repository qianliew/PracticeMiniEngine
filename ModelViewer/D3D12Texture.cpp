#include "stdafx.h"
#include "D3D12Texture.h"

UINT* D3D12Texture::GetTextureWidth()
{
    return &m_width;
}

UINT* D3D12Texture::GetTextureHeight()
{
    return &m_height;
}

UINT* D3D12Texture::GetTextureSize()
{
    return &m_size;
}

UINT64* D3D12Texture::GetTextureBytesPerRow()
{
    return &m_bytesPerRow;
}

BYTE* D3D12Texture::GetTextureData()
{
    return *m_data.get();
}

void D3D12Texture::LoadTexture(LPCWSTR texturePath)
{
    IWICImagingFactory* pFactory = NULL;
    IWICBitmapDecoder* pDecoder = NULL;
    IWICBitmapFrameDecode* pFrameDecode = NULL;
    IWICFormatConverter* pConverter = NULL;

    CoInitialize(NULL);
    ThrowIfFailed(CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory)
    ));

    ThrowIfFailed(pFactory->CreateDecoderFromFilename(texturePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder));
    ThrowIfFailed(pDecoder->GetFrame(0, &pFrameDecode));
    ThrowIfFailed(pFrameDecode->GetSize(&m_width, &m_height));

    WICPixelFormatGUID pixelFormat;
    ThrowIfFailed(pFrameDecode->GetPixelFormat(&pixelFormat));
    WICPixelFormatGUID convertToPixelFormat = GUID_WICPixelFormat32bppRGBA;
    ThrowIfFailed(pFactory->CreateFormatConverter(&pConverter));

    BOOL canConvert = FALSE;
    ThrowIfFailed(pConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert));
    ThrowIfFailed(pConverter->Initialize(pFrameDecode, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom));

    m_bytesPerRow = (m_width * 32) / 8;
    m_size = m_bytesPerRow * m_height;

    m_data = std::make_unique<BYTE*>();
    *m_data.get() = (BYTE*)malloc(m_size);
    m_dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    ThrowIfFailed(pConverter->CopyPixels(0, m_bytesPerRow, m_size, *m_data.get()));

    // Create texture desc.
    if (TextureBuffer == nullptr)
    {
        TextureBuffer = std::make_unique<D3D12TextureBuffer>();
    }

    TextureBuffer->ResourceDesc = new D3D12_RESOURCE_DESC();

    TextureBuffer->ResourceDesc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    TextureBuffer->ResourceDesc->Alignment = 0;
    TextureBuffer->ResourceDesc->Width = m_width;
    TextureBuffer->ResourceDesc->Height = m_height;
    TextureBuffer->ResourceDesc->DepthOrArraySize = 1;
    TextureBuffer->ResourceDesc->MipLevels = 1;
    TextureBuffer->ResourceDesc->Format = m_dxgiFormat;
    TextureBuffer->ResourceDesc->SampleDesc.Count = 1;
    TextureBuffer->ResourceDesc->SampleDesc.Quality = 0;
    TextureBuffer->ResourceDesc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    TextureBuffer->ResourceDesc->Flags = D3D12_RESOURCE_FLAG_NONE;
}

void D3D12Texture::ReleaseTexture()
{
    if (m_data == nullptr) return;

    m_data.release();
}
