#include "stdafx.h"
#include "D3D12Texture.h"

D3D12Texture::D3D12Texture(UINT inSRVID) :
    D3D12Texture(inSRVID, -1, 0, 0, D3D12TextureType::ShaderResource)
{

}

D3D12Texture::D3D12Texture(
    UINT inSRVID,
    UINT inIndex,
    UINT inWidth,
    UINT inHeght,
    D3D12TextureType inType,
    DXGI_FORMAT format) :
    srvID(inSRVID),
    width(inWidth),
    height(inHeght),
    type(inType),
    mipLevel(1),
    slice(1),
    srvDimension(D3D12_SRV_DIMENSION_TEXTURE2D),
    dxgiFormat(format)
{
    pTextureBuffer = nullptr;
    switch (inType)
    {
    case D3D12TextureType::RenderTarget:
        rtvHandle = inIndex;
        break;
    case D3D12TextureType::DepthStencil:
        dsvHandle = inIndex;
        break;
    case D3D12TextureType::UnorderedAccess:
        uavHandle = inIndex;
        break;
    }

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

void D3D12Texture::LoadTexture2D(std::wstring& texturePath, UINT inMipLevel)
{
    mipLevel = inMipLevel;
    srvDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    slice = 1;
    LoadSingleTexture(texturePath, 0);
}

void D3D12Texture::LoadTexture2DArray(std::vector<std::wstring>& texturePaths, UINT inMipLevel)
{
    mipLevel = inMipLevel;
    srvDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
    slice = texturePaths.size();

    for (UINT i = 0; i < texturePaths.size(); i++)
    {
        LoadSingleTexture(texturePaths[i], i);
    }
}

void D3D12Texture::LoadTextureCube(std::wstring& texturePath, UINT inMipLevel)
{
    mipLevel = 1;
    srvDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
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

void D3D12Texture::CreateTextureResource()
{
    if (type == D3D12TextureType::ShaderResource)
    {
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
        else if (srvDimension == D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
        {
            viewDesc.Texture2DArray.MostDetailedMip = 0;
            viewDesc.Texture2DArray.MipLevels = mipLevel;
            viewDesc.Texture2DArray.FirstArraySlice = 0;
            viewDesc.Texture2DArray.ArraySize = slice;
            viewDesc.Texture2DArray.PlaneSlice = 0;
            viewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
        }

        pTextureBuffer = new D3D12ShaderResourceBuffer(viewDesc);
    }
    else if (type == D3D12TextureType::RenderTarget)
    {
        D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
        viewDesc.Format = dxgiFormat;
        viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Texture2D.PlaneSlice = 0;

        pTextureBuffer = new D3D12RenderTargetBuffer(viewDesc);
    }
    else if (type == D3D12TextureType::DepthStencil)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc;
        viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
        viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        viewDesc.Flags = D3D12_DSV_FLAG_NONE;
        viewDesc.Texture2D.MipSlice = 0;

        pTextureBuffer = new D3D12DepthStencilBuffer(viewDesc);
    }
    else if (type == D3D12TextureType::UnorderedAccess)
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc;
        viewDesc.Format = dxgiFormat;
        viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        viewDesc.Texture2D.MipSlice = 0;
        viewDesc.Texture2D.PlaneSlice = 0;

        pTextureBuffer = new D3D12UnorderedAccessBuffer(viewDesc);
    }
}

const D3D12_RESOURCE_DESC D3D12Texture::GetResourceDesc()
{
    // Create the texture desc.
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = width;
    resourceDesc.Height = height;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    switch (type)
    {
    case D3D12TextureType::ShaderResource:
        resourceDesc.DepthOrArraySize = slice;
        resourceDesc.MipLevels = mipLevel;
        resourceDesc.Format = dxgiFormat;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        break;
    case D3D12TextureType::RenderTarget:
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = dxgiFormat;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        break;
    case D3D12TextureType::DepthStencil:
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        break;
    case D3D12TextureType::UnorderedAccess:
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = dxgiFormat;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        break;
    }

    return resourceDesc;
}

void D3D12Texture::ChangeTextureType(D3D12TextureType newType)
{
    type = newType;
    D3D12Resource* oldBuffer = pTextureBuffer;
    CreateTextureResource();

    if (oldBuffer != nullptr)
    {
        pTextureBuffer->SetBuffer(oldBuffer->GetBuffer());
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
void D3D12Texture::LoadSingleTexture(std::wstring& texturePath, UINT sliceIndex)
{
    UINT mipWidth = 0, mipHeight = 0, bytesPerRow = 0, indexOffset = sliceIndex * mipLevel;
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
