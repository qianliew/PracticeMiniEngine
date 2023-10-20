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

    D3D12_RESOURCE_DESC* desc = new D3D12_RESOURCE_DESC();
    desc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc->Alignment = 0;
    desc->Width = m_width;
    desc->Height = m_height;
    desc->DepthOrArraySize = 1;
    desc->MipLevels = 1;
    desc->Format = m_dxgiFormat;
    desc->SampleDesc.Count = 1;
    desc->SampleDesc.Quality = 0;
    desc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc->Flags = D3D12_RESOURCE_FLAG_NONE;

    TextureBuffer->SetResourceDesc(desc);
}

void D3D12Texture::CreateSampler()
{
    if (TextureSampler == nullptr)
    {
        TextureSampler = std::make_unique<D3D12Sampler>();
    }

    TextureSampler->SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    TextureSampler->SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    TextureSampler->SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    TextureSampler->SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    TextureSampler->SamplerDesc.MipLODBias = 0;
    TextureSampler->SamplerDesc.MaxAnisotropy = 0;
    TextureSampler->SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    TextureSampler->SamplerDesc.BorderColor[3] = 1;
    TextureSampler->SamplerDesc.MinLOD = 0.0f;
    TextureSampler->SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
}

void D3D12Texture::ReleaseTexture()
{
    if (m_data == nullptr) return;

    m_data.release();
}
