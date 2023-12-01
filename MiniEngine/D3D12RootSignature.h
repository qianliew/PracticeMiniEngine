#pragma once

// Root index in Graphic pipelines.
enum class eRootIndex
{
    ConstantBufferViewGlobal = 0,
    ConstantBufferViewPerObject,
    ShaderResourceViewGlobal0,
    ShaderResourceViewGlobal1,
    ShaderResourceViewPerObject,
    UnorderedAccessViewGlobal,
    Sampler,
    Count,
};

// Root index in DXR pipelines.
enum class eDXRRootIndex
{
    ShaderResourceViewGlobal = 0,
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

public:
    D3D12RootSignature(std::shared_ptr<D3D12Device>&);
    ~D3D12RootSignature();

    void CreateRootSignature();
    void CreateDXRRootSignature();

    inline ComPtr<ID3D12RootSignature>& GetRootSignature() { return pRootSignature; }
};
