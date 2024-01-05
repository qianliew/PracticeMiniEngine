#include "stdafx.h"
#include "FrustumCullingPass.h"
#include "CompiledShaders\FrustumCulling.hlsl.h"

FrustumCullingPass::FrustumCullingPass(
    shared_ptr<D3D12Device>& device,
    shared_ptr<SceneManager>& sceneManager,
    shared_ptr<ViewManager>& viewManager) :
    AbstractRenderPass(device, sceneManager, viewManager)
{

}

FrustumCullingPass::~FrustumCullingPass()
{
    delete pFrustumCullingData;
}

void FrustumCullingPass::Setup(D3D12CommandList* pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{
    CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };
    auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
    D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)g_pFrustumCulling, _countof(g_pFrustumCulling));
    lib->SetDXILLibrary(&libdxil);

    auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
    hitGroup->SetIntersectionShaderImport(kIntersectionShaderName);
    hitGroup->SetClosestHitShaderImport(kClosestHitShaderName);
    hitGroup->SetHitGroupExport(kHitGroupName);
    hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE);

    auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
    UINT payloadSize = sizeof(FrustumCullingRayPayload);
    UINT attributeSize = sizeof(AABBAttributes);
    shaderConfig->Config(payloadSize, attributeSize);

    CD3DX12_ROOT_SIGNATURE_DESC localRootSignatureDesc(0, nullptr);
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

    auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
    localRootSignature->SetRootSignature(pRaytracingLocalRootSignature.Get());

    auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
    rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);
    rootSignatureAssociation->AddExport(kHitGroupName);

    auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
    globalRootSignature->SetRootSignature(pRootSignature.Get());

    auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
    pipelineConfig->Config(RaytracingConstants::kMaxRayRecursiveDepth);

    // Create the state object.
    ThrowIfFailed(pDevice->GetDXRDevice()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&pDXRStateObject)));

    // Build the shader table.
    void* rayGenShaderIdentifier;
    void* missShaderIdentifier;
    void* hitGroupShaderIdentifier;

    auto GetShaderIdentifiers = [&](auto* stateObjectProperties)
    {
        rayGenShaderIdentifier = stateObjectProperties->GetShaderIdentifier(kRaygenShaderName);
        missShaderIdentifier = stateObjectProperties->GetShaderIdentifier(kMissShaderName);
        hitGroupShaderIdentifier = stateObjectProperties->GetShaderIdentifier(kHitGroupName);
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
        rayGenShaderTable.PushBack(ShaderRecord(rayGenShaderIdentifier, shaderIdentifierSize));
        pRayGenShaderTable = rayGenShaderTable.ResourceLocation.Resource;
    }

    // Miss shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable missShaderTable(numShaderRecords, shaderRecordSize);
        missShaderTable.CreateBuffer(pDevice->GetDevice().Get());
        missShaderTable.PushBack(ShaderRecord(missShaderIdentifier, shaderIdentifierSize));
        pMissShaderTable = missShaderTable.ResourceLocation.Resource;
    }

    // Hit group shader table
    {
        UINT numShaderRecords = 1;
        UINT shaderRecordSize = shaderIdentifierSize;
        ShaderTable hitGroupShaderTable(numShaderRecords, shaderRecordSize);
        hitGroupShaderTable.CreateBuffer(pDevice->GetDevice().Get());
        hitGroupShaderTable.PushBack(ShaderRecord(hitGroupShaderIdentifier, shaderIdentifierSize));
        pHitGroupShaderTable = hitGroupShaderTable.ResourceLocation.Resource;
    }

    // Create a UAV for keeping frustum culling data.
    D3D12_RESOURCE_DESC desc;

    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = GlobalConstants::kMaxNumObject;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc;
    viewDesc.Format = DXGI_FORMAT_UNKNOWN;
    viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    viewDesc.Buffer.FirstElement = 0;
    viewDesc.Buffer.NumElements = GlobalConstants::kVisDataSize;
    viewDesc.Buffer.StructureByteStride = GlobalConstants::kSizeOfUint;
    viewDesc.Buffer.CounterOffsetInBytes = 0;
    viewDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    pFrustumCullingData = new D3D12UnorderedAccessBuffer(desc, viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pFrustumCullingData,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        L"FrustumCullingData");
    pFrustumCullingData->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(UNORDERED_ACCESS_VIEW, 1));

    // Create a upload buffer to upload.
    pTempBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateUploadBuffer(pTempBuffer, desc.Width);
    pTempBuffer->CopyData(visData, desc.Width);

    pCommandList->AddTransitionResourceBarriers(pFrustumCullingData->GetResource().Get(),
        pFrustumCullingData->GetResourceState(), D3D12_RESOURCE_STATE_COPY_DEST);
    pCommandList->FlushResourceBarriers();
    pCommandList->CopyResource(
        pFrustumCullingData->GetResource().Get(),
        pTempBuffer->ResourceLocation.Resource.Get());
    pCommandList->AddTransitionResourceBarriers(pFrustumCullingData->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, pFrustumCullingData->GetResourceState());
    pCommandList->FlushResourceBarriers();

    // Create a readback buffer to read data back.
    pReadbackBuffer = new D3D12ReadbackBuffer();
    pDevice->GetBufferManager()->AllocateReadbackBuffer(pReadbackBuffer, desc.Width);
}

void FrustumCullingPass::Update()
{
    // Reset VisData.
    for (UINT i = 0; i < GlobalConstants::kVisDataSize; i++)
    {

    }
}

void FrustumCullingPass::Execute(D3D12CommandList* pCommandList)
{
    FLOAT scale = 0.05;
    UINT width = pSceneManager->GetCamera()->GetCameraWidth();
    UINT height = pSceneManager->GetCamera()->GetCameraHeight();

    auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
    {
        dispatchDesc->HitGroupTable.StartAddress = pHitGroupShaderTable->GetGPUVirtualAddress();
        dispatchDesc->HitGroupTable.SizeInBytes = pHitGroupShaderTable->GetDesc().Width;
        dispatchDesc->HitGroupTable.StrideInBytes = dispatchDesc->HitGroupTable.SizeInBytes;
        dispatchDesc->MissShaderTable.StartAddress = pMissShaderTable->GetGPUVirtualAddress();
        dispatchDesc->MissShaderTable.SizeInBytes = pMissShaderTable->GetDesc().Width;
        dispatchDesc->MissShaderTable.StrideInBytes = dispatchDesc->MissShaderTable.SizeInBytes;
        dispatchDesc->RayGenerationShaderRecord.StartAddress = pRayGenShaderTable->GetGPUVirtualAddress();
        dispatchDesc->RayGenerationShaderRecord.SizeInBytes = pRayGenShaderTable->GetDesc().Width;
        dispatchDesc->Width = width;
        dispatchDesc->Height = height;
        dispatchDesc->Depth = 1;
        commandList->SetPipelineState1(stateObject);
        commandList->DispatchRays(dispatchDesc);
    };

    // Bind resources for the frustum culling.
    pSceneManager->SetFrustumCullingResources(pCommandList);
    pCommandList->SetComputeRootUnorderedAccessView(
        (UINT)eDXRRootIndex::UnorderedAccessViewVisData,
        pFrustumCullingData->GetResource()->GetGPUVirtualAddress());

    // Bind the UAV heap for the output.
    pDevice->GetDescriptorHeapManager()->SetComputeViews(
        pCommandList->GetCommandList(),
        UNORDERED_ACCESS_VIEW,
        (UINT)eDXRRootIndex::UnorderedAccessViewGlobal,
        0);

    // Dispatch rays.    
    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    DispatchRays(pCommandList->GetDXRCommandList().Get(), pDXRStateObject.Get(), &dispatchDesc);

    // Copy the output to the color buffer.
    const D3D12Resource* pColorResource = pViewManager->GetCurrentRTVBuffer(pViewManager->GetCurrentColorHandle());
    const D3D12Resource* pOutputResource = pViewManager->GetUAVBuffer(pViewManager->GetUAVColorHandle());
    CopyBuffer(pCommandList, pColorResource, pOutputResource);

    pCommandList->AddTransitionResourceBarriers(pFrustumCullingData->GetResource().Get(),
        pFrustumCullingData->GetResourceState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    pCommandList->FlushResourceBarriers();
    pCommandList->CopyResource(
        pReadbackBuffer->ResourceLocation.Resource.Get(),
        pFrustumCullingData->GetResource().Get());
    pCommandList->AddTransitionResourceBarriers(pFrustumCullingData->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, pFrustumCullingData->GetResourceState());
    pCommandList->FlushResourceBarriers();

    pReadbackBuffer->ReadbackData(visData);
}
