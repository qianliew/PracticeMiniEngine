#include "stdafx.h"
#include "D3D12Texture.h"

D3D12Texture::D3D12Texture(UINT inSRVID, UINT inRTVID) :
    D3D12Texture(inSRVID, inRTVID, 0, 0)
{

}

D3D12Texture::D3D12Texture(UINT inSRVID, UINT inRTVID, UINT inWidth, UINT inHeght, DXGI_FORMAT format) :
    srvID(inSRVID),
    rtvID(inRTVID),
    width(inWidth),
    height(inHeght),
    mipLevel(1),
    slice(1),
    srvDimension(D3D12_SRV_DIMENSION_TEXTURE2D),
    dxgiFormat(format)
{
    pTextureBuffer = nullptr;

    // Init the loader factory.
    CoInitialize(NULL);
    ThrowIfFailed(CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory)
    ));
}

D3D12Texture::~D3D12Texture()
{
    ReleaseTextureData();
    ReleaseTextureBuffer();
}

void D3D12Texture::LoadTexture(std::wstring& texturePath, UINT inMipLevel,
    D3D12_SRV_DIMENSION inSRVDimension, UINT inSlice)
{
    mipLevel = inMipLevel;
    srvDimension = inSRVDimension;
    slice = inSlice;

    if (srvDimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
    {
        mipLevel = 1;
        slice = 6;

        std::wstring path = texturePath;
        LoadSingleTexture(path.insert(texturePath.length(), kCubemapPX), 0);
        path = texturePath;
        LoadSingleTexture(path.insert(texturePath.length(), kCubemapNX), 1);
        path = texturePath;
        LoadSingleTexture(path.insert(texturePath.length(), kCubemapPY), 2);
        path = texturePath;
        LoadSingleTexture(path.insert(texturePath.length(), kCubemapNY), 3);
        path = texturePath;
        LoadSingleTexture(path.insert(texturePath.length(), kCubemapPZ), 4);
        path = texturePath;
        LoadSingleTexture(path.insert(texturePath.length(), kCubemapNZ), 5);
    }
    else
    {
        LoadSingleTexture(texturePath, 0);
    }
}

void D3D12Texture::CreateTexture(D3D12TextureType inType)
{
    type = inType;
    D3D12Resource* oldBuffer = pTextureBuffer;

    // Create texture desc.
    D3D12_RESOURCE_DESC desc;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = slice;
    desc.MipLevels = mipLevel;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    if (type == D3D12TextureType::ShaderResource)
    {
        desc.Format = dxgiFormat;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = dxgiFormat == DXGI_FORMAT_R32_TYPELESS ? DXGI_FORMAT_R32_FLOAT : dxgiFormat;
        viewDesc.ViewDimension = srvDimension;
        viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (srvDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
        {
            viewDesc.Texture2D.MostDetailedMip = 0;
            viewDesc.Texture2D.MipLevels = mipLevel;
            viewDesc.Texture2D.PlaneSlice = 0;
            viewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
        }
        else if (srvDimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
        {
            viewDesc.TextureCube.MostDetailedMip = 0;
            viewDesc.TextureCube.MipLevels = mipLevel;
            viewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
        }

        pTextureBuffer = new D3D12ShaderResourceBuffer(desc, viewDesc);
    }
    else if (type == D3D12TextureType::RenderTarget)
    {
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
        viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Texture2D.PlaneSlice = 0;

        pTextureBuffer = new D3D12RenderTargetBuffer(desc, viewDesc);
    }
    else if (type == D3D12TextureType::DepthStencil)
    {
        desc.Format = DXGI_FORMAT_D32_FLOAT;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc;
        viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Flags = D3D12_DSV_FLAG_NONE;
        viewDesc.Texture2D.MipSlice = 0;

        pTextureBuffer = new D3D12DepthStencilBuffer(desc, viewDesc);
    }
    else if (type == D3D12TextureType::UnorderedAccess)
    {
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc;
        viewDesc.Format = dxgiFormat;
        viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Texture2D.PlaneSlice = 0;

        pTextureBuffer = new D3D12UnorderedAccessBuffer(desc, viewDesc);
    }

    if (oldBuffer != nullptr)
    {
        pTextureBuffer->SetResourceLoaction(oldBuffer->GetResource());
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

// Helper functions
void D3D12Texture::LoadSingleTexture(std::wstring& texturePath, UINT index)
{
    UINT mipWidth = 0, mipHeight = 0, bytesPerRow = 0, indexOffset = index * mipLevel;
    UINT64 size = 0;
    for (int i = 0; i < mipLevel; i++)
    {
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(GetTexturePath(texturePath, i).c_str())
            && GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            ThrowIfFailed(pFactory->CreateDecoderFromFilename(GetDefaultMipTexturePath(texturePath, mipWidth).c_str(),
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
        pData[i + indexOffset] = bytes;

        if (mipWidth == 0) break;
    }

    pFactory->Release();
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

std::wstring D3D12Texture::GetDefaultMipTexturePath(std::wstring texturePath, UINT mipSize)
{
    std::wstring path = texturePath.find(L"_mra") == texturePath.size() - 8 ? kDefaultMRATexturePath :
        texturePath.find(L"_n") == texturePath.size() - 6 ? kDefaultNormalTexturePath :
        kDefaultTexturePath;
    path.insert(path.find_last_of(L'.'), std::to_wstring(mipSize));

    return path;
}
