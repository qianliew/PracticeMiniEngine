#include "stdafx.h"
#include "D3D12ShaderTable.h"

ShaderTable::ShaderTable(UINT numShaderRecords, UINT inShaderRecordSize) :
	shaderRecordSize(Align(inShaderRecordSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT))
{
    shaderRecords.reserve(numShaderRecords);
    bufferSize = numShaderRecords * shaderRecordSize;
}

void ShaderTable::CreateBuffer(
	ID3D12Device* device,
	UINT64 size,
	D3D12_RESOURCE_STATES state,
	const wchar_t* name)
{
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
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
