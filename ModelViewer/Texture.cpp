#include "stdafx.h"
#include "Texture.h"

Texture::~Texture()
{
    ReleaseTexture();
}

UINT Texture::GetTextureWidth()
{
    return m_width;
}

UINT Texture::GetTextureHeight()
{
    return m_height;
}

UINT Texture::GetTextureSize()
{
    return m_size;
}

UINT Texture::GetTextureBytesPerRow()
{
    return m_bytesPerRow;
}

BYTE* Texture::GetTextureData()
{
    return *m_data.get();
}

D3D12_RESOURCE_DESC* Texture::GetTextureDesc()
{
    return m_desc;
}

void Texture::LoadTexture(LPCWSTR texturePath)
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
    m_desc = new D3D12_RESOURCE_DESC();

    m_desc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    m_desc->Alignment = 0;
    m_desc->Width = m_width;
    m_desc->Height = m_height;
    m_desc->DepthOrArraySize = 1;
    m_desc->MipLevels = 1;
    m_desc->Format = m_dxgiFormat;
    m_desc->SampleDesc.Count = 1;
    m_desc->SampleDesc.Quality = 0;
    m_desc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    m_desc->Flags = D3D12_RESOURCE_FLAG_NONE;
}

void Texture::ReleaseTexture()
{
    m_data.release();
    delete m_desc;
    m_desc = nullptr;
}
