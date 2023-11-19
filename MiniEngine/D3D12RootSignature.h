#pragma once

class D3D12RootSignature
{
private:
    std::shared_ptr<D3D12Device>& pDevice;
    ComPtr<ID3D12RootSignature> pRootSignature;

public:
    D3D12RootSignature(std::shared_ptr<D3D12Device>&);
    ~D3D12RootSignature();

    void Create();

    inline ComPtr<ID3D12RootSignature>& GetRootSignature() { return pRootSignature; }
};
