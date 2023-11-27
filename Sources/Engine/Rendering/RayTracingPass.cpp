#include "stdafx.h"
#include "RayTracingPass.h"
#include "CompiledShaders\Raytracing.hlsl.h"

RayTracingPass::RayTracingPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager) :
    pDevice(device),
    pSceneManager(sceneManager)
{

}

void RayTracingPass::Setup(D3D12CommandList*& pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

    // DXIL library
    // This contains the shaders and their entrypoints for the state object.
    // Since shaders are not considered a subobject, they need to be passed in via DXIL library subobjects.
    auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pRaytracing, ARRAYSIZE(g_pRaytracing));
    lib->SetDXILLibrary(&libdxil);

    // Triangle hit group
    // A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
    // In this sample, we only use triangle geometry with a closest hit shader, so others are not set.
    auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(kClosestHitShaderName);
    hitGroup->SetHitGroupExport(kHitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(kAOClosestHitShaderName);
    hitGroup->SetHitGroupExport(kAOHitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(kGIClosestHitShaderName);
    hitGroup->SetHitGroupExport(kGIHitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // Shader config
    // Defines the maximum sizes in bytes for the ray payload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = max(sizeof(RayPayload), sizeof(AORayPayload), sizeof(GIRayPayload));
    UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
    shaderConfig->Config(payloadSize, attributeSize);

    // Local Root Signature
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    {
        CD3DX12_ROOT_PARAMETER rootParameters[1];
        rootParameters[0].InitAsConstants(32, 1, 0);
        CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(ARRAYSIZE(rootParameters), rootParameters);
        localRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

        ComPtr<ID3DBlob> blob;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(
            &localRootSignatureDesc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &blob,
            &error));

        ThrowIfFailed(pDevice->GetDevice()->CreateRootSignature(1,
            blob->GetBufferPointer(),
            blob->GetBufferSize(),
            IID_PPV_ARGS(&pRaytracingLocalRootSignature)));
    }

    // Hit group and miss shaders in this sample are not using a local root signature and thus one is not associated with them.

    // Local root signature to be used in a ray gen shader.
    {
        auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(pRaytracingLocalRootSignature.Get());
        // Shader association
        auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
        rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
        rootSignatureAssociation->AddExport(kRaygenShaderName);
    }

    // Global root signature
    // This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(pRootSignature.Get());

    // Pipeline config
    // Defines the maximum TraceRay() recursion depth.
    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    // PERFOMANCE TIP: Set max recursion depth as low as needed 
    // as drivers may apply optimization strategies for low recursion depths. 
    UINT maxRecursionDepth = RaytracingConstants::MaxRayRecursiveDepth; // ~ primary rays only. 
    pipelineConfig->Config(maxRecursionDepth);

    // Create the state object.
    ThrowIfFailed(pDevice->GetDXRDevice()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&pDXRStateObject)));
}

void RayTracingPass::BuildShaderTables()
{
    void* rayGenShaderIdentifier[1];
    void* missShaderIdentifier[RayType::Count];
    void* hitGroupShaderIdentifier[RayType::Count];

    auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
    {
        rayGenShaderIdentifier[0] = stateObjectProperties->GetShaderIdentifier(kRaygenShaderName);
        hitGroupShaderIdentifier[RayType::Radiance] = stateObjectProperties->GetShaderIdentifier(kHitGroupName);
        hitGroupShaderIdentifier[RayType::AO] = stateObjectProperties->GetShaderIdentifier(kAOHitGroupName);
        hitGroupShaderIdentifier[RayType::GI] = stateObjectProperties->GetShaderIdentifier(kGIHitGroupName);
        missShaderIdentifier[RayType::Radiance] = stateObjectProperties->GetShaderIdentifier(kMissShaderName);
        missShaderIdentifier[RayType::AO] = stateObjectProperties->GetShaderIdentifier(kAOMissShaderName);
        missShaderIdentifier[RayType::GI] = stateObjectProperties->GetShaderIdentifier(kGIMissShaderName);
    };

    // Get shader identifiers.
    UINT shaderIdentifierSize;
    {
        ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
        ThrowIfFailed(pDXRStateObject.As(&stateObjectProperties));
        GetShaderIdentifiers(stateObjectProperties.Get());
        shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    }

    // Ray gen shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable rayGenShaderTable(numShaderRecords, shaderRecordSize);
        rayGenShaderTable.CreateBuffer(pDevice->GetDevice().Get());
        rayGenShaderTable.PushBack(ShaderRecord(rayGenShaderIdentifier[0], shaderIdentifierSize));
        pRayGenShaderTable = rayGenShaderTable.ResourceLocation.Resource;
    }

    // Miss shader table
    {
        UINT numShaderRecords = RayType::Count;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable missShaderTable(numShaderRecords, shaderRecordSize);
        missShaderTable.CreateBuffer(pDevice->GetDevice().Get());
        for (UINT i = 0; i < RayType::Count; i++)
        {
            missShaderTable.PushBack(ShaderRecord(missShaderIdentifier[i], shaderIdentifierSize));
        }
        pMissShaderTable = missShaderTable.ResourceLocation.Resource;
    }

    // Hit group shader table
    {
        UINT numShaderRecords = RayType::Count;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable hitGroupShaderTable(numShaderRecords, shaderRecordSize);
        hitGroupShaderTable.CreateBuffer(pDevice->GetDevice().Get());
        for (UINT i = 0; i < RayType::Count; i++)
        {
            hitGroupShaderTable.PushBack(ShaderRecord(hitGroupShaderIdentifier[i], shaderIdentifierSize));
        }
        pHitGroupShaderTable = hitGroupShaderTable.ResourceLocation.Resource;
    }
}

void RayTracingPass::Execute(D3D12CommandList*& pCommandList, UINT frameIndex)
{
    auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
    {
        // Since each shader table has only one shader record, the stride is same as the size.
        dispatchDesc->HitGroupTable.StartAddress = pHitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc->HitGroupTable.SizeInBytes = pHitGroupShaderTable->GetDesc().Width;
        dispatchDesc->HitGroupTable.StrideInBytes = dispatchDesc->HitGroupTable.SizeInBytes / RayType::Count;
        dispatchDesc->MissShaderTable.StartAddress = pMissShaderTable->GetGPUVirtualAddress();
        dispatchDesc->MissShaderTable.SizeInBytes = pMissShaderTable->GetDesc().Width;
        dispatchDesc->MissShaderTable.StrideInBytes = dispatchDesc->MissShaderTable.SizeInBytes / RayType::Count;
        dispatchDesc->RayGenerationShaderRecord.StartAddress = pRayGenShaderTable->GetGPUVirtualAddress();
        dispatchDesc->RayGenerationShaderRecord.SizeInBytes = pRayGenShaderTable->GetDesc().Width;
        dispatchDesc->Width = 1920;
        dispatchDesc->Height = 1080;
        dispatchDesc->Depth = 1;
        commandList->SetPipelineState1(stateObject);
        commandList->DispatchRays(dispatchDesc);
    };

    pSceneManager->SetDXRResources(pCommandList);

    pCommandList->SetComputeRootConstantBufferView(
        (UINT)eDXRRootIndex::ConstantBufferViewGlobal,
        pDevice->GetBufferManager()->GetGlobalConstantBuffer()->GetResource()->GetGPUVirtualAddress());

    // Bind the UAV heap for the output.
    pDevice->GetDescriptorHeapManager()->SetComputeViews(
        pCommandList->GetCommandList(),
        UNORDERED_ACCESS_VIEW,
        (UINT)eDXRRootIndex::UnorderedAccessViewGlobal,
        0);

    // Dispatch rays.    
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    DispatchRays(pCommandList->GetDXRCommandList().Get(), pDXRStateObject.Get(), &dispatchDesc);
}
