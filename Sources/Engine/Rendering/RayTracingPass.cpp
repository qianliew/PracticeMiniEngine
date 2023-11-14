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
    // Define which shader exports to surface from the library.
    // If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
    // In this sample, this could be omitted for convenience since the sample uses all shaders in the library. 
    {
        lib->DefineExport(kRaygenShaderName);
        lib->DefineExport(kClosestHitShaderName);
        lib->DefineExport(kMissShaderName);
    }

    // Triangle hit group
    // A hit group specifies closest hit, any hit and intersection shaders to be executed when a ray intersects the geometry's triangle/AABB.
    // In this sample, we only use triangle geometry with a closest hit shader, so others are not set.
    auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetClosestHitShaderImport(kClosestHitShaderName);
    hitGroup->SetHitGroupExport(kHitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

    // Shader config
    // Defines the maximum sizes in bytes for the ray payload and attribute structure.
    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = 4 * sizeof(float);   // float4 color
    UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
    shaderConfig->Config(payloadSize, attributeSize);

    // Local Root Signature
    // This is a root signature that enables a shader to have unique arguments that come from shader tables.
    {
        CD3DX12_ROOT_PARAMETER rootParameters[1];
        rootParameters[0].InitAsConstants(SizeOfInUint32(rayGenCB), 0, 0);
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
            IID_PPV_ARGS(&RaytracingLocalRootSignature)));
    }

    // Hit group and miss shaders in this sample are not using a local root signature and thus one is not associated with them.

    // Local root signature to be used in a ray gen shader.
    {
        auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
        localRootSignature->SetRootSignature(RaytracingLocalRootSignature.Get());
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
    UINT maxRecursionDepth = 1; // ~ primary rays only. 
    pipelineConfig->Config(maxRecursionDepth);

    // Create the state object.
    ThrowIfFailed(pDevice->GetDXRDevice()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&DXRStateObject)));
}
