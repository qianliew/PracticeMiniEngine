#pragma once

// Root index in Graphic pipelines.
enum class eRootIndex
{
    ConstantBufferViewGlobal = 0,
    ConstantBufferViewPerObject,
    ShaderResourceViewGlobal0,
    ShaderResourceViewGlobal1,
    ShaderResourceViewGlobal2,
    ShaderResourceViewPerObject,
    ShaderResourceViewGBuffer,
    UnorderedAccessViewGlobal,
    Sampler,
    Count,
};

// Root index in DXR pipelines.
enum class eDXRRootIndex
{
    ShaderResourceViewTLAS = 0,
    ShaderResourceViewIndex,
    ShaderResourceViewVertex,
    ShaderResourceViewOffset,
    ShaderResourceViewCommandBuffer,
    ShaderResourceViewSkybox,
    ShaderResourceViewDepth,
    ShaderResourceViewColor,
    Sampler,
    ConstantBufferViewGlobal,
    UnorderedAccessViewGlobal,
    UnorderedAccessViewVisData,
    UnorderedAccessViewCommandBuffer,
    Count,
};

class D3D12RootSignature
{
private:
    std::shared_ptr<D3D12Device>& pDevice;
    ComPtr<ID3D12RootSignature> pRootSignature;
    ComPtr<ID3D12RootSignature> pDRXRootSignature;
    D3D12_STATIC_SAMPLER_DESC staticSamplerDesc;

public:
    D3D12RootSignature(std::shared_ptr<D3D12Device>&);
    ~D3D12RootSignature();

    void CreateRootSignature();
    void CreateDXRRootSignature();

    inline ComPtr<ID3D12RootSignature>& GetRootSignature() { return pRootSignature; }
    inline ComPtr<ID3D12RootSignature>& GetDRXRootSignature() { return pDRXRootSignature; }
};
