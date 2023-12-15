#include "stdafx.h"
#include "D3D12RootSignature.h"

D3D12RootSignature::D3D12RootSignature(std::shared_ptr<D3D12Device>& inDevice) :
    pDevice(inDevice)
{

}

D3D12RootSignature::~D3D12RootSignature()
{

}

void D3D12RootSignature::CreateRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE descriptorTableRanges[7];
    descriptorTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    descriptorTableRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);
    descriptorTableRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);
    descriptorTableRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 5, 0);
    descriptorTableRanges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 10, 0);
    descriptorTableRanges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
    descriptorTableRanges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 3, 5, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[(UINT)eRootIndex::Count];
    rootParameters[(UINT)eRootIndex::ConstantBufferViewGlobal].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[(UINT)eRootIndex::ConstantBufferViewPerObject].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[(UINT)eRootIndex::ShaderResourceViewGlobal0].InitAsDescriptorTable(1, &descriptorTableRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[(UINT)eRootIndex::ShaderResourceViewGlobal1].InitAsDescriptorTable(1, &descriptorTableRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[(UINT)eRootIndex::ShaderResourceViewGlobal2].InitAsDescriptorTable(1, &descriptorTableRanges[2], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[(UINT)eRootIndex::ShaderResourceViewPerObject].InitAsDescriptorTable(1, &descriptorTableRanges[3], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[(UINT)eRootIndex::ShaderResourceViewGBuffer].InitAsDescriptorTable(1, &descriptorTableRanges[4], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[(UINT)eRootIndex::UnorderedAccessViewGlobal].InitAsDescriptorTable(1, &descriptorTableRanges[5], D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[(UINT)eRootIndex::Sampler].InitAsDescriptorTable(1, &descriptorTableRanges[6], D3D12_SHADER_VISIBILITY_PIXEL);

    // create a static sampler
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = 0.0f;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        signature.GetAddressOf(),
        error.GetAddressOf()));
    ThrowIfFailed(pDevice->GetDevice()->CreateRootSignature(0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&pRootSignature)));
}

void D3D12RootSignature::CreateDXRRootSignature()
{
    CD3DX12_DESCRIPTOR_RANGE descriptorTableRanges[4];
    descriptorTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
    descriptorTableRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4);
    descriptorTableRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 5);
    descriptorTableRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 2, 5);

    CD3DX12_ROOT_PARAMETER rootParameters[(UINT)eDXRRootIndex::Count];
    rootParameters[(UINT)eDXRRootIndex::ShaderResourceViewTLAS].InitAsShaderResourceView(0);
    rootParameters[(UINT)eDXRRootIndex::ShaderResourceViewIndex].InitAsShaderResourceView(1);
    rootParameters[(UINT)eDXRRootIndex::ShaderResourceViewVertex].InitAsShaderResourceView(2);
    rootParameters[(UINT)eDXRRootIndex::ShaderResourceViewOffset].InitAsShaderResourceView(3);
    rootParameters[(UINT)eDXRRootIndex::ShaderResourceViewSkybox].InitAsDescriptorTable(1, &descriptorTableRanges[1]);
    rootParameters[(UINT)eDXRRootIndex::ShaderResourceViewTexture].InitAsDescriptorTable(1, &descriptorTableRanges[2]);
    rootParameters[(UINT)eDXRRootIndex::Sampler].InitAsDescriptorTable(1, &descriptorTableRanges[3]);
    rootParameters[(UINT)eDXRRootIndex::ConstantBufferViewGlobal].InitAsConstantBufferView(0);
    rootParameters[(UINT)eDXRRootIndex::UnorderedAccessViewGlobal].InitAsDescriptorTable(1, &descriptorTableRanges[0]);
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        signature.GetAddressOf(),
        error.GetAddressOf()));
    ThrowIfFailed(pDevice->GetDevice()->CreateRootSignature(1,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&pDRXRootSignature)));
}
