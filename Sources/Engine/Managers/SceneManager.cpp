#include "stdafx.h"
#include "SceneManager.h"
#include "LitMaterial.h"
#include "SkyboxMaterial.h"

UINT SceneManager::sTextureID = 0;

SceneManager::SceneManager(shared_ptr<D3D12Device>& device) :
    pDevice(device),
    objectID(0)
{
    const UINT64 size = 1024 * 1024;
    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
    pTempVertexBuffer = new D3D12UploadBuffer(resourceDesc);
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pTempVertexBuffer);
    pTempIndexBuffer = new D3D12UploadBuffer(resourceDesc);
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pTempIndexBuffer);
    pTempOffsetBuffer = new D3D12UploadBuffer(resourceDesc);
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pTempOffsetBuffer);
    pTempBoundingBoxBuffer = new D3D12UploadBuffer(resourceDesc);
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pTempBoundingBoxBuffer);
}

SceneManager::~SceneManager()
{
    UnloadScene();

    delete pSkyboxMaterial;
    delete pSkyboxMesh;
    // delete pFullScreenMesh;
    delete pCamera;

    delete pVertexBuffer;
    delete pIndexBuffer;
    delete pOffsetBuffer;
    delete pFrustumCullingData;
}

void SceneManager::InitFBXImporter()
{
    pFBXImporter.release();
    pFBXImporter = std::make_unique<FBXImporter>();
    pFBXImporter->InitializeSdkObjects();
}

void SceneManager::ParseScene(D3D12CommandList* pCommandList)
{
    LPCWSTR sceneName = L"scene";
    std::wifstream inFile(GetAssetPath(sceneName));

    // Parse textures from the scene file to materials.
    UINT numMaterials = 0;
    inFile >> numMaterials;
    for (UINT i = 0; i < numMaterials; i++)
    {
        WCHAR materialName[32];
        inFile >> materialName;
        LitMaterial* material = new LitMaterial(materialName);
        material->LoadTexture();

        LoadTextureBufferAndSampler(pCommandList, material->GetTexture());
        LoadTextureBufferAndSampler(pCommandList, material->GetMRATexture());
        LoadTextureBufferAndSampler(pCommandList, material->GetNormalTexture());
        pMaterialPool[EraseSuffix(materialName)] = material;
    }

    // Parse FBX from the scene file.
    UINT numModels = 0, offset = 0;
    inFile >> numModels;

    for (UINT i = 0; i < numModels; i++)
    {
        WCHAR fileName[32];
        inFile >> fileName;

        Model* model = new Model(objectID++, fileName);
        model->LoadModel(pFBXImporter);
        model->SetMaterial(pMaterialPool[EraseSuffix(fileName)]);
        AddObject(model);

        LoadObjectVertexBufferAndIndexBufferDXR(pCommandList, model, offset);
        LoadObjectVertexBufferAndIndexBuffer(pCommandList, model);
    }

    // Create the SRV of indices and vertices.
    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(pTempIndexBuffer->GetBufferSize());
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    pIndexBuffer = new D3D12ShaderResourceBuffer(srvDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pIndexBuffer,
        resourceDesc,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        L"IndexBuffer");
    pCommandList->CopyBufferRegion(
        pIndexBuffer,
        pTempIndexBuffer->GetResource().Get(),
        pTempIndexBuffer->GetBufferUsage());

    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(pTempVertexBuffer->GetBufferSize());
    srvDesc = {};
    pVertexBuffer = new D3D12ShaderResourceBuffer(srvDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pVertexBuffer,
        resourceDesc,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        L"VertexBuffer");
    pCommandList->CopyBufferRegion(
        pVertexBuffer,
        pTempVertexBuffer->GetResource().Get(),
        pTempVertexBuffer->GetBufferUsage());

    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(pTempOffsetBuffer->GetBufferSize());
    srvDesc = {};
    pOffsetBuffer = new D3D12ShaderResourceBuffer(srvDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pOffsetBuffer,
        resourceDesc,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        L"OffsetBuffer");
    pCommandList->CopyBufferRegion(
        pOffsetBuffer,
        pTempOffsetBuffer->GetResource().Get(),
        pTempOffsetBuffer->GetBufferUsage());

    BuildBottomLevelAS(pCommandList, GeometryType::Triangle);
    BuildBottomLevelAS(pCommandList, GeometryType::AABB);
    BuildTopLevelAS(pCommandList, GeometryType::Triangle);
    BuildTopLevelAS(pCommandList, GeometryType::AABB);

    inFile.close();
}

void SceneManager::LoadScene(D3D12CommandList* pCommandList, ComPtr<ID3D12RootSignature>& pRootSignature)
{
    // Parse the scene file.
    ParseScene(pCommandList);

    // Create static data.
    pFullScreenMesh = new Model(objectID++, L"");
    pFullScreenMesh->CreatePlane();
    LoadObjectVertexBufferAndIndexBuffer(pCommandList, pFullScreenMesh);

    // Create assets of the skybox.
    std::wstring skyboxName = L"Skybox\\sky01";
    SkyboxMaterial* material = new SkyboxMaterial(skyboxName);
    material->LoadTexture();
    LoadTextureBufferAndSampler(pCommandList, material->GetTexture());
    pSkyboxMaterial = material;

    pSkyboxMesh = new Model(objectID++, L"Skybox\\skybox.fbx");
    pSkyboxMesh->LoadModel(pFBXImporter);
    LoadObjectVertexBufferAndIndexBuffer(pCommandList, pSkyboxMesh);

    // Create the global constant buffer.
    pDevice->GetBufferManager()->AllocateGlobalConstantBuffer();
    pDevice->GetBufferManager()->GetGlobalConstantBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(CONSTANT_BUFFER_VIEW_GLOBAL, 0));

    // Create a UAV for keeping frustum culling data.
    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
        GlobalConstants::kMaxNumObject,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
    pFrustumCullingData = new D3D12UnorderedAccessBuffer(viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pFrustumCullingData,
        resourceDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        L"FrustumCullingData");

    // Create a upload buffer to upload and reset the vis data.
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    pUploadBuffer = new D3D12UploadBuffer(resourceDesc);
    pDevice->GetBufferManager()->AllocateUploadBuffer(
        pUploadBuffer,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        L"FrustumCullingUploadBuffer");
    ResetVisData(pCommandList);

    // Create a readback buffer to read data back.
    pReadbackBuffer = new D3D12ReadbackBuffer(resourceDesc);
    pDevice->GetBufferManager()->AllocateReadbackBuffer(
        pReadbackBuffer,
        D3D12_RESOURCE_STATE_COPY_DEST,
        L"FrustumCullingReadbackBuffer");

    // Create a command signature for indirect drawing.
    D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
    argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
    argumentDescs[0].ConstantBufferView.RootParameterIndex = (UINT)eCommandSignatureIndex::CBV;
    argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

    D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
    commandSignatureDesc.pArgumentDescs = argumentDescs;
    commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
    commandSignatureDesc.ByteStride = sizeof(IndirectCommand);

    ThrowIfFailed(pDevice->GetDevice()->CreateCommandSignature(
        &commandSignatureDesc,
        pRootSignature.Get(),
        IID_PPV_ARGS(&pCommandSignature)));

    // Create the command buffer.
    std::vector<IndirectCommand> commands;
    commands.resize(GlobalConstants::kVisDataSize);
    for (UINT i = 0; i < pObjects.size(); i++)
    {
        Model* model = pObjects[i];
        UINT id = pObjects[i]->GetObjectID();
        commands[i].cbv = pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(id)->GetResource()->GetGPUVirtualAddress();
        commands[i].drawArguments.VertexCountPerInstance = model->GetMesh()->GetIndexCount();
        commands[i].drawArguments.InstanceCount = 1;
        commands[i].drawArguments.StartVertexLocation = 0;
        commands[i].drawArguments.StartInstanceLocation = 0;
    }

    D3D12_RESOURCE_DESC commandBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(commandBufferSize);
    D3D12_SHADER_RESOURCE_VIEW_DESC commandBufferSRVDesc = {};

    pTempCommandBuffer = new D3D12UploadBuffer(commandBufferDesc);
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pTempCommandBuffer);
    pTempCommandBuffer->CopyData(&commands.data()[0], commandBufferSize);

    pCommandBuffer = std::make_shared<D3D12ShaderResourceBuffer>(commandBufferSRVDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pCommandBuffer.get(),
        commandBufferDesc,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        L"CommandBuffer");
    pCommandList->CopyResource(pCommandBuffer.get(), pTempCommandBuffer->GetResource().Get());

    // Create the processed command buffer
    D3D12_RESOURCE_DESC argumentBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
        argumentBufferSize + sizeof(UINT),
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    D3D12_UNORDERED_ACCESS_VIEW_DESC argumentBufferUAVDesc = {};
    argumentBufferUAVDesc.Format = DXGI_FORMAT_UNKNOWN;
    argumentBufferUAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    argumentBufferUAVDesc.Buffer.FirstElement = 0;
    argumentBufferUAVDesc.Buffer.NumElements = GlobalConstants::kVisDataSize;
    argumentBufferUAVDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);
    argumentBufferUAVDesc.Buffer.CounterOffsetInBytes = argumentBufferSize;
    argumentBufferUAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    pArgumentBuffer = std::make_shared<D3D12UnorderedAccessBuffer>(argumentBufferUAVDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        pArgumentBuffer.get(),
        argumentBufferDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        L"ArgumentBuffer");
    pArgumentBuffer->SetCounterResource();
    pArgumentBuffer->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(UNORDERED_ACCESS_VIEW, 1));

    D3D12_RESOURCE_DESC countBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT));
    pCountBuffer = new D3D12UploadBuffer(countBufferDesc);
    pDevice->GetBufferManager()->AllocateUploadBuffer(pCountBuffer);
    pCountBuffer->ZeroData(sizeof(UINT));

    pCommandList->CopyBufferRegion(
        pArgumentBuffer.get(),
        pCountBuffer->GetResource().Get(),
        sizeof(UINT),
        argumentBufferSize);
}

void SceneManager::UnloadScene()
{
    objectID = 0;
    sTextureID = 0;

    for (auto it = pObjects.begin(); it != pObjects.end(); it++)
    {
        delete* it;
    }
    pObjects.clear();

    for (auto it = pMaterialPool.begin(); it != pMaterialPool.end(); it++)
    {
        delete it->second;
    }
    pMaterialPool.clear();
}

void SceneManager::CreateCamera(UINT width, UINT height)
{
    pCamera = new Camera(0, static_cast<FLOAT>(width), static_cast<FLOAT>(height));
    pCamera->SetViewport(static_cast<FLOAT>(width), static_cast<FLOAT>(height));
    pCamera->SetScissorRect(static_cast<LONG>(width), static_cast<LONG>(height));
}

void SceneManager::AddObject(Model* object)
{
	pObjects.push_back(object);
}

void SceneManager::DrawObjects(D3D12CommandList* pCommandList)
{
    for (UINT i = 0; i < pObjects.size(); i++)
    {
        Model* model = pObjects[i];
        UINT id = pObjects[i]->GetObjectID();
        if (visData[id] == 0) continue;

        // Set the per object views.
        pCommandList->SetRootConstantBufferView((UINT)eRootIndex::ConstantBufferViewPerObject,
            pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(id)->GetResource()->GetGPUVirtualAddress());

        // Set the material relating views.
        LitMaterial* litMaterial = dynamic_cast<LitMaterial*>(model->GetMaterial());

        pDevice->GetDescriptorHeapManager()->SetViews(
            pCommandList->GetCommandList(),
            SHADER_RESOURCE_VIEW_PEROBJECT,
            (UINT)eRootIndex::ShaderResourceViewPerObject,
            litMaterial->GetTexture()->GetTextureID());
        pDevice->GetDescriptorHeapManager()->SetViews(
            pCommandList->GetCommandList(),
            SAMPLER,
            (UINT)eRootIndex::Sampler,
            litMaterial->GetTexture()->GetTextureID());

        // Set buffers and draw the instance.
        pCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pCommandList->SetVertexBuffers(0, 1, &model->GetMesh()->GetVertexBufferView());
        pCommandList->SetIndexBuffer(&model->GetMesh()->GetIndexBufferView());
        pCommandList->DrawIndexedInstanced(model->GetMesh()->GetIndexCount());
    }

    ResetVisData(pCommandList);
}

void SceneManager::DrawObjectsIndirectly(D3D12CommandList* pCommandList)
{
    pCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pCommandList->ExecuteIndirect(
        pCommandSignature.Get(),
        pObjects.size(),
        pArgumentBuffer->GetResource().Get(),
        0,
        pArgumentBuffer->GetResource().Get(),
        argumentBufferSize);
}

void SceneManager::DrawSkybox(D3D12CommandList* pCommandList)
{
    // Set the global CBV.
    UINT id = pSkyboxMesh->GetObjectID();
    pCommandList->SetRootConstantBufferView(CONSTANT_BUFFER_VIEW_PEROBJECT,
        pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(id)->GetResource()->GetGPUVirtualAddress());
    
    // Set SRVs.
    pDevice->GetDescriptorHeapManager()->SetViews(
        pCommandList->GetCommandList(),
        SHADER_RESOURCE_VIEW_PEROBJECT,
        (UINT)eRootIndex::ShaderResourceViewPerObject,
        pSkyboxMaterial->GetTexture()->GetTextureID());
    pDevice->GetDescriptorHeapManager()->SetViews(
        pCommandList->GetCommandList(),
        SAMPLER,
        (UINT)eRootIndex::Sampler,
        pSkyboxMaterial->GetTexture()->GetTextureID());

    pCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pCommandList->SetVertexBuffers(0, 1, &pSkyboxMesh->GetMesh()->GetVertexBufferView());
    pCommandList->SetIndexBuffer(&pSkyboxMesh->GetMesh()->GetIndexBufferView());
    pCommandList->DrawIndexedInstanced(pSkyboxMesh->GetMesh()->GetIndexCount());
}

void SceneManager::DrawFullScreenMesh(D3D12CommandList* pCommandList)
{
    pCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pCommandList->SetVertexBuffers(0, 1, &pFullScreenMesh->GetMesh()->GetVertexBufferView());
    pCommandList->SetIndexBuffer(&pFullScreenMesh->GetMesh()->GetIndexBufferView());
    pCommandList->DrawIndexedInstanced(pFullScreenMesh->GetMesh()->GetIndexCount());
}

void SceneManager::SetFrustumCullingResources(D3D12CommandList* pCommandList)
{
    // Bind the heap of TLAS.
    pCommandList->SetComputeRootShaderResourceView(
        (UINT)eDXRRootIndex::ShaderResourceViewTLAS,
        tlas[GeometryType::AABB].pTopLevelAccelerationStructure->GetResource()->GetGPUVirtualAddress());

    // Bind the vis data.
    pCommandList->SetComputeRootUnorderedAccessView(
        (UINT)eDXRRootIndex::UnorderedAccessViewVisData,
        pFrustumCullingData->GetResource()->GetGPUVirtualAddress());

    // Bind the command buffer.
    pCommandList->SetComputeRootShaderResourceView(
        (UINT)eDXRRootIndex::ShaderResourceViewCommandBuffer,
        pCommandBuffer->GetResource()->GetGPUVirtualAddress());
    pDevice->GetDescriptorHeapManager()->SetComputeViews(
        pCommandList->GetCommandList(),
        UNORDERED_ACCESS_VIEW,
        (UINT)eDXRRootIndex::UnorderedAccessViewArgumentBuffer,
        1);
}

void SceneManager::ReadbackFrustumCullingData(D3D12CommandList* pCommandList)
{
    pCommandList->CopyResource(pReadbackBuffer->GetResource().Get(), pFrustumCullingData);
    pReadbackBuffer->ReadbackData(visData, sizeof(visData));
}

void SceneManager::SetDXRResources(D3D12CommandList* pCommandList)
{
    // Bind the heap of TLAS.
    pCommandList->SetComputeRootShaderResourceView(
        (UINT)eDXRRootIndex::ShaderResourceViewTLAS,
        tlas[GeometryType::Triangle].pTopLevelAccelerationStructure->GetResource()->GetGPUVirtualAddress());

    // Bind the global heaps.
    pCommandList->SetComputeRootShaderResourceView(
        (UINT)eDXRRootIndex::ShaderResourceViewIndex,
        pIndexBuffer->GetResource()->GetGPUVirtualAddress());
    pCommandList->SetComputeRootShaderResourceView(
        (UINT)eDXRRootIndex::ShaderResourceViewVertex,
        pVertexBuffer->GetResource()->GetGPUVirtualAddress());
    pCommandList->SetComputeRootShaderResourceView(
        (UINT)eDXRRootIndex::ShaderResourceViewOffset,
        pOffsetBuffer->GetResource()->GetGPUVirtualAddress());

    // Bind textures.
    pDevice->GetDescriptorHeapManager()->SetComputeViews(
        pCommandList->GetCommandList(),
        SHADER_RESOURCE_VIEW_PEROBJECT,
        (UINT)eDXRRootIndex::ShaderResourceViewSkybox,
        pSkyboxMaterial->GetTexture()->GetTextureID());
}

void SceneManager::UpdateScene()
{

}

void SceneManager::UpdateTransforms()
{
    // Set the transform of the skybox.
    pCamera->SetObjectToWorldMatrix();
    pSkyboxMesh->CopyWorldPosition(*pCamera);
    pSkyboxMesh->SetObjectToWorldMatrix();
    pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(pSkyboxMesh->GetObjectID())
        ->CopyData(&pSkyboxMesh->GetTransformConstant(), sizeof(TransformConstant));

    // Set the transform of objects.
    for (UINT i = 0; i < pObjects.size(); i++)
    {
        pObjects[i]->SetObjectToWorldMatrix();
        pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(pObjects[i]->GetObjectID())
            ->CopyData(&pObjects[i]->GetTransformConstant(), sizeof(TransformConstant));
    }
}

void SceneManager::UpdateCamera()
{
    pCamera->UpdateCameraConstant();
    pDevice->GetBufferManager()->GetGlobalConstantBuffer()->CopyData(&pCamera->GetCameraConstant(), sizeof(CameraConstant));
}

void SceneManager::Release()
{
    for (auto it = pMaterialPool.begin(); it != pMaterialPool.end(); it++)
    {
        if (it->second != nullptr)
        {
            it->second->ReleaseTextureData();
        }
    }
}

// Helper functions.
void SceneManager::LoadObjectVertexBufferAndIndexBuffer(D3D12CommandList* pCommandList, Model* object)
{
    // Create the perObject constant buffer and its view.
    UINT id = object->GetObjectID();
    pDevice->GetBufferManager()->AllocatePerObjectConstantBuffers(id);
    pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(id)->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(CONSTANT_BUFFER_VIEW_PEROBJECT, id));

    // Create the vertex buffer and index buffer and their view.
    D3D12_RESOURCE_DESC resourceBuffer = CD3DX12_RESOURCE_DESC::Buffer(object->GetMesh()->GetVertexSize());
    D3D12UploadBuffer* tempVertexBuffer = new D3D12UploadBuffer(resourceBuffer);
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(tempVertexBuffer);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        object->GetMesh()->GetVertexBuffer(),
        object->GetMesh()->GetVertexResourceDesc(),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    tempVertexBuffer->CopyData(object->GetMesh()->GetVertexData(), object->GetMesh()->GetVertexSize());

    resourceBuffer = CD3DX12_RESOURCE_DESC::Buffer(object->GetMesh()->GetIndexSize());
    D3D12UploadBuffer* tempIndexBuffer = new D3D12UploadBuffer(resourceBuffer);
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(tempIndexBuffer);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        object->GetMesh()->GetIndexBuffer(),
        object->GetMesh()->GetIndexResourceDesc(),
        D3D12_RESOURCE_STATE_INDEX_BUFFER);
    tempIndexBuffer->CopyData(object->GetMesh()->GetIndexData(), object->GetMesh()->GetIndexSize());
    object->GetMesh()->CreateView();

    pCommandList->CopyBufferRegion(
        object->GetMesh()->GetVertexBuffer(),
        tempVertexBuffer->GetResource().Get(),
        object->GetMesh()->GetVertexSize());
    pCommandList->CopyBufferRegion(
        object->GetMesh()->GetIndexBuffer(),
        tempIndexBuffer->GetResource().Get(),
        object->GetMesh()->GetIndexSize());
}

void SceneManager::LoadObjectVertexBufferAndIndexBufferDXR(D3D12CommandList* pCommandList, Model* object, UINT& offset)
{
    // Create the geometry desc for this object.
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    geometryDesc.Triangles.Transform3x4 = 0;
    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.IndexCount = object->GetMesh()->GetIndexCount();
    geometryDesc.Triangles.VertexCount = object->GetMesh()->GetVertexCount();
    geometryDesc.Triangles.IndexBuffer =
        pTempIndexBuffer->GetResource()->GetGPUVirtualAddress() + pTempIndexBuffer->GetBufferUsage();
    geometryDesc.Triangles.VertexBuffer.StartAddress =
        pTempVertexBuffer->GetResource()->GetGPUVirtualAddress() + pTempVertexBuffer->GetBufferUsage();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
    blas[GeometryType::Triangle].geometryDescs.push_back(geometryDesc);

    // Copy vertex and index data to a common buffer.
    pTempVertexBuffer->CopyData(
        object->GetMesh()->GetVertexData(),
        object->GetMesh()->GetVertexSize(),
        pTempVertexBuffer->GetBufferUsage());

    pTempIndexBuffer->CopyData(
        object->GetMesh()->GetIndexData(),
        object->GetMesh()->GetIndexSize(),
        pTempIndexBuffer->GetBufferUsage());

    pTempOffsetBuffer->CopyData(
        &offset,
        sizeof(UINT),
        pTempOffsetBuffer->GetBufferUsage());
    offset += object->GetMesh()->GetIndexCount();

    // Create the geomerty desc for the binding box of this object.
    geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    geometryDesc.AABBs.AABBCount = 1;
    geometryDesc.AABBs.AABBs.StartAddress =
        pTempBoundingBoxBuffer->GetResource()->GetGPUVirtualAddress() + pTempBoundingBoxBuffer->GetBufferUsage();
    geometryDesc.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
    blas[GeometryType::AABB].geometryDescs.push_back(geometryDesc);

    pTempBoundingBoxBuffer->CopyData(
        &object->GetAABBBox()->GetData(),
        sizeof(D3D12_RAYTRACING_AABB),
        pTempBoundingBoxBuffer->GetBufferUsage());
}

void SceneManager::LoadTextureBufferAndSampler(D3D12CommandList* pCommandList, D3D12Texture* texture)
{
    UINT id = texture->GetTextureID();

    // Create the texture buffer.
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        texture->GetTextureBuffer(),
        texture->GetResourceDesc(),
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    texture->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(SHADER_RESOURCE_VIEW_PEROBJECT, id));

    // Init texture data.
    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(0);
    for (UINT i = 0; i < texture->GetSubresourceNum(); i++)
    {
        D3D12_SUBRESOURCE_DATA textureData;
        UINT64 rowSizeInBytes, totalBytes;
        pDevice->GetDevice()->GetCopyableFootprints(&texture->GetTextureBuffer()->GetResourceDesc(),
            i, 1, 0, nullptr, nullptr, &rowSizeInBytes, &totalBytes);
        textureData.pData = texture->GetTextureDataAt(i);
        textureData.RowPitch = rowSizeInBytes;
        textureData.SlicePitch = totalBytes;

        resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);
        D3D12UploadBuffer* tempBuffer = new D3D12UploadBuffer(resourceDesc);
        pDevice->GetBufferManager()->AllocateTempUploadBuffer(tempBuffer);

        // Update texture data from upload buffer to gpu buffer.
        pCommandList->CopyTextureBuffer(texture->GetTextureBuffer(), tempBuffer->GetResource().Get(), 0, i, 1, &textureData);
    }

    // Create the sampler and its view.
    texture->CreateSampler();
    texture->TextureSampler->CPUHandle = pDevice->GetDescriptorHeapManager()->GetHandle(SAMPLER, id);
    pDevice->GetDevice()->CreateSampler(&texture->TextureSampler->SamplerDesc,
        texture->TextureSampler->CPUHandle);
}

void SceneManager::BuildBottomLevelAS(D3D12CommandList* pCommandList, UINT index)
{
    // Create the input of BLAS.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = {};
    bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    bottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    bottomLevelInputs.NumDescs = blas[index].geometryDescs.size();
    bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    bottomLevelInputs.pGeometryDescs = blas[index].geometryDescs.data();

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
    pDevice->GetDXRDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
    ThrowIfFalse(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
        bottomLevelPrebuildInfo.ScratchDataSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
    blas[index].pScratchResource = std::make_shared<D3D12UnorderedAccessBuffer>(viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        blas[index].pScratchResource.get(),
        resourceDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        L"ScratchResource");

    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
        bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    blas[index].pBottomLevelAccelerationStructure = std::make_shared<D3D12UnorderedAccessBuffer>(viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        blas[index].pBottomLevelAccelerationStructure.get(),
        resourceDesc,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        L"BottomLevelAccelerationStructure");

    // Bottom Level Acceleration Structure desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
    bottomLevelBuildDesc.DestAccelerationStructureData =
        blas[index].pBottomLevelAccelerationStructure->GetResource()->GetGPUVirtualAddress();
    bottomLevelBuildDesc.Inputs = bottomLevelInputs;
    bottomLevelBuildDesc.SourceAccelerationStructureData = NULL;
    bottomLevelBuildDesc.ScratchAccelerationStructureData =
        blas[index].pScratchResource->GetResource()->GetGPUVirtualAddress();

    pCommandList->GetDXRCommandList()->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
    pCommandList->GetCommandList()->ResourceBarrier(1,
        &CD3DX12_RESOURCE_BARRIER::UAV(blas[index].pBottomLevelAccelerationStructure->GetResource().Get()));
}

void SceneManager::BuildTopLevelAS(D3D12CommandList* pCommandList, UINT index)
{
    // Create instance descs for the bottom-level acceleration structure.
    const UINT64 instanceDescsSize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC);
    D3D12_RESOURCE_DESC instanceResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceDescsSize);
    pInstanceDescBuffer = new D3D12UploadBuffer(instanceResourceDesc);
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pInstanceDescBuffer);

    D3D12_RAYTRACING_INSTANCE_DESC desc = {};
    desc.Transform[0][0] = desc.Transform[1][1] = desc.Transform[2][2] = 1;
    desc.InstanceID = 0;
    desc.InstanceMask = 0xFF;
    desc.InstanceContributionToHitGroupIndex = 0;
    desc.Flags = 0;
    desc.AccelerationStructure = blas[index].pBottomLevelAccelerationStructure->GetResource()->GetGPUVirtualAddress();
    pInstanceDescBuffer->CopyData(&desc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC), 0);

    // Create the input of TLAS.
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
    topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    topLevelInputs.NumDescs =  1;
    topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    topLevelInputs.InstanceDescs = pInstanceDescBuffer->GetResource()->GetGPUVirtualAddress();

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    pDevice->GetDXRDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
        topLevelPrebuildInfo.ScratchDataSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
    tlas[index].pScratchResource = std::make_shared<D3D12UnorderedAccessBuffer>(viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        tlas[index].pScratchResource.get(),
        resourceDesc,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        L"ScratchResource");

    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
        topLevelPrebuildInfo.ScratchDataSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    tlas[index].pTopLevelAccelerationStructure = std::make_shared<D3D12UnorderedAccessBuffer>(viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        tlas[index].pTopLevelAccelerationStructure.get(),
        resourceDesc,
        D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        L"TopLevelAccelerationStructure");

    // Top Level Acceleration Structure desc
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
    topLevelBuildDesc.DestAccelerationStructureData = 
        tlas[index].pTopLevelAccelerationStructure->GetResource()->GetGPUVirtualAddress();
    topLevelBuildDesc.Inputs = topLevelInputs;
    topLevelBuildDesc.SourceAccelerationStructureData = NULL;
    topLevelBuildDesc.ScratchAccelerationStructureData =
        tlas[index].pScratchResource->GetResource()->GetGPUVirtualAddress();

    // Build acceleration structure.
    pCommandList->GetDXRCommandList()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
}

void SceneManager::ResetVisData(D3D12CommandList* pCommandList)
{
    // Reset VisData.
    for (UINT i = 0; i < GlobalConstants::kVisDataSize; i++)
    {
        visData[i] = 0;
    }

    pUploadBuffer->CopyData(visData, sizeof(visData));
    pCommandList->CopyResource(pFrustumCullingData, pUploadBuffer->GetResource().Get());
}
