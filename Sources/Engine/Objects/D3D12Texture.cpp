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
    dxgiFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
{
    pTextureBuffer = nullptr;
}

D3D12Texture::~D3D12Texture()
{
    ReleaseTextureData();
    ReleaseTextureBuffer();
}

void D3D12Texture::LoadTexture(std::wstring& texturePath)
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

    UINT mipWidth = 0, mipHeight = 0, bytesPerRow = 0;
    UINT64 size = 0;
    for (int i = 0; i < kMipCount; i++)
    {
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(GetTexturePath(texturePath, i).c_str())
            && GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            ThrowIfFailed(pFactory->CreateDecoderFromFilename(GetDefaultTexturePath(mipWidth).c_str(),
                NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder));
        }
        else
        {
            ThrowIfFailed(pFactory->CreateDecoderFromFilename(GetTexturePath(texturePath, i).c_str(),
                NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder));
        }

        ThrowIfFailed(pDecoder->GetFrame(0, &pFrameDecode));
        ThrowIfFailed(pFrameDecode->GetSize(&mipWidth, &mipHeight));
        if (i == 0)
        {
            width = mipWidth;
            height = mipHeight;
        }

        WICPixelFormatGUID pixelFormat;
        ThrowIfFailed(pFrameDecode->GetPixelFormat(&pixelFormat));
        WICPixelFormatGUID convertToPixelFormat = GUID_WICPixelFormat32bppRGBA;
        ThrowIfFailed(pFactory->CreateFormatConverter(&pConverter));

        BOOL canConvert = FALSE;
        ThrowIfFailed(pConverter->CanConvert(pixelFormat, convertToPixelFormat, &canConvert));
        ThrowIfFailed(pConverter->Initialize(pFrameDecode, convertToPixelFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom));

        bytesPerRow = (mipWidth * 32) / 8;
        size = bytesPerRow * mipHeight;

        BYTE* bytes = new BYTE();
        bytes = (BYTE*)malloc(size);

        ThrowIfFailed(pConverter->CopyPixels(0, bytesPerRow, size, bytes));

        mipWidth /= 2;
        mipHeight /= 2;
        pData[i] = bytes;

        if (mipWidth == 0) break;
    }

    pFactory->Release();
}

void D3D12Texture::CreateTexture(D3D12TextureType inType, BOOL hasMip)
{
    type = inType;
    D3D12Resource* oldBuffer = pTextureBuffer;

    // Create texture desc.
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = hasMip ? GetMipCount() : 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    if (type == D3D12TextureType::ShaderResource)
    {
        desc.Format = dxgiFormat;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        pTextureBuffer = new D3D12TextureBuffer(desc);
    }
    else if (type == D3D12TextureType::RenderTarget)
    {
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        pTextureBuffer = new D3D12RenderTargetBuffer(desc);
    }
    else if (type == D3D12TextureType::DepthStencil)
    {
        desc.Format = DXGI_FORMAT_D32_FLOAT;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        pTextureBuffer = new D3D12DepthStencilBuffer(desc);
    }

    if (oldBuffer != nullptr)
    {
        pTextureBuffer->SetResourceLoaction(oldBuffer->GetResourceLocation());
        delete oldBuffer;
    }
}

void D3D12Texture::ReleaseTextureData()
{
    for (auto it = pData.begin(); it != pData.end(); it++)
    {
        delete it->second;
    }
    pData.clear();
}

void D3D12Texture::ReleaseTextureBuffer()
{
    delete pTextureBuffer;
}

void D3D12Texture::CreateSampler()
{
    if (TextureSampler == nullptr)
    {
        TextureSampler = std::make_unique<D3D12Sampler>();
    }

    TextureSampler->SamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    TextureSampler->SamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    TextureSampler->SamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    TextureSampler->SamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    TextureSampler->SamplerDesc.MipLODBias = 0;
    TextureSampler->SamplerDesc.MaxAnisotropy = 16;
    TextureSampler->SamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    TextureSampler->SamplerDesc.MinLOD = 0.0f;
    TextureSampler->SamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
}

std::wstring D3D12Texture::GetTexturePath(std::wstring texturePath, UINT mipIndex)
{
    if (mipIndex != 0)
    {
        std::wstring suffix = kMipSuffix;
        suffix.append(std::to_wstring(mipIndex));
        texturePath.insert(texturePath.find_last_of(L'.'), suffix);
    }

    return texturePath;
}

std::wstring D3D12Texture::GetDefaultTexturePath(UINT mipSize)
{
    std::wstring path = kDefaultTexturePath;
    path.insert(path.find_last_of(L'.'), std::to_wstring(mipSize));

    return path;
}
