#include "stdafx.h"
#include "D3D12ShaderTable.h"

ShaderTable::ShaderTable(const D3D12_RESOURCE_DESC& desc, const UINT inShaderRecordSize, UINT numShaderRecords) :
	shaderRecordSize(inShaderRecordSize),
	D3D12UploadBuffer(desc)
{
    shaderRecords.reserve(numShaderRecords);
}

void ShaderTable::CreateBuffer(
	ID3D12Device* device,
	const wchar_t* name,
	const D3D12_CLEAR_VALUE* clearValue,
	const D3D12_RESOURCE_STATES state)
{
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(pResource.GetAddressOf())));

	CD3DX12_RANGE readRange(0, 0);
	ThrowIfFailed(pResource->Map(0, &readRange, reinterpret_cast<void**>(&pMappedShaderRecords)));
}

void ShaderTable::PushBack(const ShaderRecord& shaderRecord)
{
    ThrowIfFalse(shaderRecords.size() < shaderRecords.capacity());
    shaderRecords.push_back(shaderRecord);
    shaderRecord.CopyTo(pMappedShaderRecords);
    pMappedShaderRecords += shaderRecordSize;
}
