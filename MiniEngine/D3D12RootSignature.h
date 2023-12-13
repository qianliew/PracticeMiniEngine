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
    ShaderResourceViewSkybox,
    ShaderResourceViewTexture,
    Sampler,
    ConstantBufferViewGlobal,
    UnorderedAccessViewGlobal,
    Count,
};

class D3D12RootSignature
{
private:
    std::shared_ptr<D3D12Device>& pDevice;
    ComPtr<ID3D12RootSignature> pRootSignature;
    ComPtr<ID3D12RootSignature> pDRXRootSignature;

public:
    D3D12RootSignature(std::shared_ptr<D3D12Device>&);
    ~D3D12RootSignature();

    void CreateRootSignature();
    void CreateDXRRootSignature();

    inline ComPtr<ID3D12RootSignature>& GetRootSignature() { return pRootSignature; }
    inline ComPtr<ID3D12RootSignature>& GetDRXRootSignature() { return pDRXRootSignature; }
};
