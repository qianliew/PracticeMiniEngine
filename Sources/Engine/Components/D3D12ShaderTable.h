#pragma once
#include "D3D12UploadBuffer.h"

// Shader record = {{Shader ID}, {RootArguments}}
class ShaderRecord
{
public:
    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize) :
        shaderIdentifier(pShaderIdentifier, shaderIdentifierSize)
    {
    }

    ShaderRecord(void* pShaderIdentifier, UINT shaderIdentifierSize, void* pLocalRootArguments, UINT localRootArgumentsSize) :
        shaderIdentifier(pShaderIdentifier, shaderIdentifierSize),
        localRootArguments(pLocalRootArguments, localRootArgumentsSize)
    {
    }

    void CopyTo(void* dest) const
    {
        uint8_t* byteDest = static_cast<uint8_t*>(dest);
        memcpy(byteDest, shaderIdentifier.ptr, shaderIdentifier.size);
        if (localRootArguments.ptr)
        {
            memcpy(byteDest + shaderIdentifier.size, localRootArguments.ptr, localRootArguments.size);
        }
    }

    struct PointerWithSize
    {
        void* ptr;
        UINT size;

        PointerWithSize() : ptr(nullptr), size(0) {}
        PointerWithSize(void* _ptr, UINT _size) : ptr(_ptr), size(_size) {};
    };
    PointerWithSize shaderIdentifier;
    PointerWithSize localRootArguments;
};

class ShaderTable : public D3D12UploadBuffer
{
    const UINT shaderRecordSize;

    uint8_t* pMappedShaderRecords;
    std::vector<ShaderRecord> shaderRecords;

public:
    ShaderTable(UINT numShaderRecords, UINT shaderRecordSize);

    virtual void CreateBuffer(
        ID3D12Device* device,
        UINT64 size = 0,
        D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON,
        const wchar_t* name = nullptr) override;
    void PushBack(const ShaderRecord& shaderRecord);
};
