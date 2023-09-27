#include "stdafx.h"
#include "D3D12TextureBuffer.h"

D3D12TextureBuffer::~D3D12TextureBuffer()
{
    ReleaseTexture();

    delete View;
}

UINT* D3D12TextureBuffer::GetTextureWidth()
{
    return &m_width;
}

UINT* D3D12TextureBuffer::GetTextureHeight()
{
    return &m_height;
}

UINT* D3D12TextureBuffer::GetTextureSize()
{
    return &m_size;
}

UINT64* D3D12TextureBuffer::GetTextureBytesPerRow()
{
    return &m_bytesPerRow;
}

BYTE* D3D12TextureBuffer::GetTextureData()
{
    return *m_data.get();
}

void D3D12TextureBuffer::LoadTexture(LPCWSTR texturePath)
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
    ResourceDesc = new D3D12_RESOURCE_DESC();

    ResourceDesc->Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    ResourceDesc->Alignment = 0;
    ResourceDesc->Width = m_width;
    ResourceDesc->Height = m_height;
    ResourceDesc->DepthOrArraySize = 1;
    ResourceDesc->MipLevels = 1;
    ResourceDesc->Format = m_dxgiFormat;
    ResourceDesc->SampleDesc.Count = 1;
    ResourceDesc->SampleDesc.Quality = 0;
    ResourceDesc->Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    ResourceDesc->Flags = D3D12_RESOURCE_FLAG_NONE;
}

void D3D12TextureBuffer::CreateView()
{
    if (View == nullptr)
    {
        View = new D3D12SRV();
        View->SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        View->SRVDesc.Format = ResourceDesc->Format;
        View->SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        View->SRVDesc.Texture2D.MipLevels = 1;
    }
}

void D3D12TextureBuffer::ReleaseTexture()
{
    if (m_data == nullptr) return;

    m_data.release();
}
