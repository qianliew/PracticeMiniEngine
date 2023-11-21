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
    CD3DX12_DESCRIPTOR_RANGE descriptorTableRanges[4];
    descriptorTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
    descriptorTableRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, 0);
    descriptorTableRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
    descriptorTableRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 3, 1, 0);

    CD3DX12_ROOT_PARAMETER rootParameters[6];
    rootParameters[CONSTANT_BUFFER_VIEW_GLOBAL].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[CONSTANT_BUFFER_VIEW_PEROBJECT].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[SHADER_RESOURCE_VIEW_GLOBAL].InitAsDescriptorTable(1, &descriptorTableRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[SHADER_RESOURCE_VIEW_PEROBJECT].InitAsDescriptorTable(1, &descriptorTableRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[UNORDERED_ACCESS_VIEW].InitAsDescriptorTable(1, &descriptorTableRanges[2], D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters[SAMPLER].InitAsDescriptorTable(1, &descriptorTableRanges[3], D3D12_SHADER_VISIBILITY_PIXEL);

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
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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
    CD3DX12_DESCRIPTOR_RANGE descriptorTableRanges[2];
    descriptorTableRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
    descriptorTableRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);

    CD3DX12_ROOT_PARAMETER rootParameters[4];
    rootParameters[DXR_UNORDERED_ACCESS_VIEW].InitAsDescriptorTable(1, &descriptorTableRanges[0]);
    rootParameters[DXR_SHADER_RESOURCE_VIEW_GLOBAL].InitAsShaderResourceView(0);
    rootParameters[DXR_SHADER_RESOURCE_VIEW_PEROBJECT].InitAsDescriptorTable(1, &descriptorTableRanges[1]);
    rootParameters[DXR_CONSTANT_BUFFER_VIEW_GLOBAL].InitAsConstantBufferView(0);
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
        IID_PPV_ARGS(&pRootSignature)));
}
