#include "stdafx.h"
#include "D3D12Texture.h"

D3D12Texture::D3D12Texture(UINT inID) :
    D3D12Texture(inID, 0, 0)
{

}

D3D12Texture::D3D12Texture(UINT inID, UINT inWidth, UINT inHeght) :
    id(inID),
    width(inWidth),
    height(inHeght),
    size(0),
    bytesPerRow(0),
    dxgiFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
{
    TextureBuffer = nullptr;
}

D3D12Texture::~D3D12Texture()
{
    ReleaseTexture();
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
    ThrowIfFailed(pFrameDecode->GetSize(&width, &height));

    WICPixelFormatGUID pixelFormat;
    ThrowIfFailed(pFrameDecode->GetPixelFormat(&pixelFormat));
    WICPixelFormatGUID convertToPixelFormat = GUID_WICPixelFormat32bppRGBA;
    ThrowIfFailed(pFactory->CreateFormatConverter(&pConverter));

    BOOL canConvert = FALSE;
    ThrowIfFailed(pConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert));
    ThrowIfFailed(pConverter->Initialize(pFrameDecode, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom));

    bytesPerRow = (width * 32) / 8;
    size = bytesPerRow * height;

    pData = new BYTE();
    pData = (BYTE*)malloc(size);
    dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    ThrowIfFailed(pConverter->CopyPixels(0, bytesPerRow, size, pData));
}

void D3D12Texture::CreateTexture(D3D12TextureType type)
{
    D3D12Resource* oldBuffer = TextureBuffer;

    // Create texture desc.
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    if (type == D3D12TextureType::ShaderResource)
    {
        desc.Format = dxgiFormat;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        TextureBuffer = new D3D12TextureBuffer(desc);
    }
    else if (type == D3D12TextureType::RenderTarget)
    {
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        TextureBuffer = new D3D12RenderTargetBuffer(desc);
    }
    else if (type == D3D12TextureType::DepthStencil)
    {
        desc.Format = DXGI_FORMAT_D32_FLOAT;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        TextureBuffer = new D3D12DepthStencilBuffer(desc);
    }

    if (oldBuffer != nullptr)
    {
        TextureBuffer->SetResourceLoaction(oldBuffer->GetResourceLocation());
        delete oldBuffer;
    }
}

void D3D12Texture::ReleaseTexture()
{
    delete TextureBuffer;
    delete pData;
}

void D3D12Texture::CreateSampler()
{
    if (TextureSampler == nullptr)
    {
        TextureSampler = std::make_unique<D3D12Sampler>();
    }

    TextureSampler->SamplerDesc.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    TextureSampler->SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    TextureSampler->SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    TextureSampler->SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
    TextureSampler->SamplerDesc.MipLODBias = 0;
    TextureSampler->SamplerDesc.MaxAnisotropy = 0;
    TextureSampler->SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    TextureSampler->SamplerDesc.BorderColor[0] = 0.0f;
    TextureSampler->SamplerDesc.BorderColor[1] = 0.2f;
    TextureSampler->SamplerDesc.BorderColor[2] = 0.4f;
    TextureSampler->SamplerDesc.BorderColor[3] = 1;
    TextureSampler->SamplerDesc.MinLOD = 0.0f;
    TextureSampler->SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
}
