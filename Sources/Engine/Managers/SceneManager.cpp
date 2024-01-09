#include "stdafx.h"
#include "SceneManager.h"
#include "LitMaterial.h"
#include "SkyboxMaterial.h"

UINT SceneManager::sTextureID = 0;

SceneManager::SceneManager(shared_ptr<D3D12Device>& device, BOOL isDXR) :
    pDevice(device),
    objectID(0)
{
    pTempVertexBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pTempVertexBuffer, 1024 * 1024);
    pTempIndexBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pTempIndexBuffer, 1024 * 1024);
    pTempOffsetBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pTempOffsetBuffer, 1024);
    pTempBoundingBoxBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pTempBoundingBoxBuffer, 1024);
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
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = pTempIndexBuffer->GetBufferUsage() / sizeof(UINT16);
    srvDesc.Buffer.StructureByteStride = sizeof(Vertex);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    pIndexBuffer = new D3D12ShaderResourceBuffer(resourceDesc, srvDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(pIndexBuffer);
    pIndexBuffer->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(SHADER_RESOURCE_VIEW_GLOBAL, 1));
    pCommandList->CopyBufferRegion(pIndexBuffer->GetResource().Get(),
        pTempIndexBuffer->ResourceLocation.Resource.Get(),
        pTempIndexBuffer->GetBufferUsage());

    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(pTempVertexBuffer->GetBufferSize());
    srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = pTempVertexBuffer->GetBufferUsage() / sizeof(Vertex);
    srvDesc.Buffer.StructureByteStride = sizeof(UINT16);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    pVertexBuffer = new D3D12ShaderResourceBuffer(resourceDesc, srvDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(pVertexBuffer);
    pVertexBuffer->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(SHADER_RESOURCE_VIEW_GLOBAL, 2));
    pCommandList->CopyBufferRegion(pVertexBuffer->GetResource().Get(),
        pTempVertexBuffer->ResourceLocation.Resource.Get(),
        pTempVertexBuffer->GetBufferUsage());

    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(pTempOffsetBuffer->GetBufferSize());
    srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements = numModels;
    srvDesc.Buffer.StructureByteStride = sizeof(UINT);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    pOffsetBuffer = new D3D12ShaderResourceBuffer(resourceDesc, srvDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(pOffsetBuffer);
    pOffsetBuffer->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(SHADER_RESOURCE_VIEW_GLOBAL, 3));
    pCommandList->CopyBufferRegion(pOffsetBuffer->GetResource().Get(),
        pTempOffsetBuffer->ResourceLocation.Resource.Get(),
        pTempOffsetBuffer->GetBufferUsage());

    BuildBottomLevelAS(pCommandList, GeometryType::Triangle);
    BuildBottomLevelAS(pCommandList, GeometryType::AABB);
    BuildTopLevelAS(pCommandList, GeometryType::Triangle);
    BuildTopLevelAS(pCommandList, GeometryType::AABB);

    inFile.close();
}

void SceneManager::LoadScene(D3D12CommandList* pCommandList)
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

    // Create a upload buffer to upload and reset the vis data.
    pUploadBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateUploadBuffer(pUploadBuffer, desc.Width);
    ResetVisData(pCommandList);

    // Create a readback buffer to read data back.
    pReadbackBuffer = new D3D12ReadbackBuffer();
    pDevice->GetBufferManager()->AllocateReadbackBuffer(pReadbackBuffer, desc.Width);
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
        pCommandList->SetVertexBuffers(0, 1, &model->GetMesh()->GetVertexBuffer()->VertexBufferView);
        pCommandList->SetIndexBuffer(&model->GetMesh()->GetIndexBuffer()->IndexBufferView);
        pCommandList->DrawIndexedInstanced(model->GetMesh()->GetIndicesNum());
    }

    ResetVisData(pCommandList);
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

    pCommandList->SetVertexBuffers(0, 1, &pSkyboxMesh->GetMesh()->GetVertexBuffer()->VertexBufferView);
    pCommandList->SetIndexBuffer(&pSkyboxMesh->GetMesh()->GetIndexBuffer()->IndexBufferView);
    pCommandList->DrawIndexedInstanced(pSkyboxMesh->GetMesh()->GetIndicesNum());
}

void SceneManager::DrawFullScreenMesh(D3D12CommandList* pCommandList)
{
    pCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pCommandList->SetVertexBuffers(0, 1, &pFullScreenMesh->GetMesh()->GetVertexBuffer()->VertexBufferView);
    pCommandList->SetIndexBuffer(&pFullScreenMesh->GetMesh()->GetIndexBuffer()->IndexBufferView);
    pCommandList->DrawIndexedInstanced(pFullScreenMesh->GetMesh()->GetIndicesNum());
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
}

void SceneManager::ReadbackFrustumCullingData(D3D12CommandList* pCommandList)
{
    pCommandList->AddTransitionResourceBarriers(pFrustumCullingData->GetResource().Get(),
        pFrustumCullingData->GetResourceState(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    pCommandList->FlushResourceBarriers();
    pCommandList->CopyResource(
        pReadbackBuffer->ResourceLocation.Resource.Get(),
        pFrustumCullingData->GetResource().Get());
    pCommandList->AddTransitionResourceBarriers(pFrustumCullingData->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_SOURCE, pFrustumCullingData->GetResourceState());
    pCommandList->FlushResourceBarriers();

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
    D3D12UploadBuffer* tempVertexBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(tempVertexBuffer, object->GetMesh()->GetVerticesSize());
    pDevice->GetBufferManager()->AllocateDefaultBuffer(object->GetMesh()->GetVertexBuffer());
    tempVertexBuffer->CopyData(object->GetMesh()->GetVerticesData(), object->GetMesh()->GetVerticesSize());

    D3D12UploadBuffer* tempIndexBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(tempIndexBuffer, object->GetMesh()->GetIndicesSize());
    pDevice->GetBufferManager()->AllocateDefaultBuffer(object->GetMesh()->GetIndexBuffer());
    tempIndexBuffer->CopyData(object->GetMesh()->GetIndicesData(), object->GetMesh()->GetIndicesSize());

    object->GetMesh()->CreateView();
    pCommandList->CopyBufferRegion(object->GetMesh()->GetVertexBuffer()->GetResource().Get(),
        tempVertexBuffer->ResourceLocation.Resource.Get(),
        object->GetMesh()->GetVerticesSize());
    pCommandList->CopyBufferRegion(object->GetMesh()->GetIndexBuffer()->GetResource().Get(),
        tempIndexBuffer->ResourceLocation.Resource.Get(),
        object->GetMesh()->GetIndicesSize());

    // Setup transition barriers.
    pCommandList->AddTransitionResourceBarriers(object->GetMesh()->GetVertexBuffer()->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    pCommandList->AddTransitionResourceBarriers(object->GetMesh()->GetIndexBuffer()->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    pCommandList->FlushResourceBarriers();
}

void SceneManager::LoadObjectVertexBufferAndIndexBufferDXR(D3D12CommandList* pCommandList, Model* object, UINT& offset)
{
    // Create the perObject constant buffer and its view.
    UINT id = object->GetObjectID();
    pDevice->GetBufferManager()->AllocatePerObjectConstantBuffers(id);
    pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(id)->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(CONSTANT_BUFFER_VIEW_PEROBJECT, id));

    // Create the geometry desc for this object.
    D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

    geometryDesc.Triangles.Transform3x4 = 0;
    geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
    geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    geometryDesc.Triangles.IndexCount = object->GetMesh()->GetIndicesNum();
    geometryDesc.Triangles.VertexCount = object->GetMesh()->GetVerticesNum();
    geometryDesc.Triangles.IndexBuffer =
        pTempIndexBuffer->ResourceLocation.Resource->GetGPUVirtualAddress() + pTempIndexBuffer->GetBufferUsage();
    geometryDesc.Triangles.VertexBuffer.StartAddress =
        pTempVertexBuffer->ResourceLocation.Resource->GetGPUVirtualAddress() + pTempVertexBuffer->GetBufferUsage();
    geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
    blas[GeometryType::Triangle].geometryDescs.push_back(geometryDesc);

    // Copy vertex and index data to a common buffer.
    pTempVertexBuffer->CopyData(
        object->GetMesh()->GetVerticesData(),
        object->GetMesh()->GetVerticesSize(),
        pTempVertexBuffer->GetBufferUsage());

    pTempIndexBuffer->CopyData(
        object->GetMesh()->GetIndicesData(),
        object->GetMesh()->GetIndicesSize(),
        pTempIndexBuffer->GetBufferUsage());

    pTempOffsetBuffer->CopyData(
        &offset,
        sizeof(UINT),
        pTempOffsetBuffer->GetBufferUsage());
    offset += object->GetMesh()->GetIndicesNum();

    // Create the geomerty desc for the binding box of this object.
    geometryDesc = {};
    geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
    geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
    geometryDesc.AABBs.AABBCount = 1;
    geometryDesc.AABBs.AABBs.StartAddress =
        pTempBoundingBoxBuffer->ResourceLocation.Resource->GetGPUVirtualAddress() + pTempBoundingBoxBuffer->GetBufferUsage();
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
    pDevice->GetBufferManager()->AllocateDefaultBuffer(texture->GetTextureBuffer());
    texture->GetTextureBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(SHADER_RESOURCE_VIEW_PEROBJECT, id));

    // Init texture data.
    for (UINT i = 0; i < texture->GetSubresourceNum(); i++)
    {
        D3D12_SUBRESOURCE_DATA textureData;
        UINT64 rowSizeInBytes, totalBytes;
        pDevice->GetDevice()->GetCopyableFootprints(&texture->GetTextureBuffer()->GetResourceDesc(),
            i, 1, 0, nullptr, nullptr, &rowSizeInBytes, &totalBytes);
        textureData.pData = texture->GetTextureDataAt(i);
        textureData.RowPitch = rowSizeInBytes;
        textureData.SlicePitch = totalBytes;

        D3D12UploadBuffer* tempBuffer = new D3D12UploadBuffer();
        pDevice->GetBufferManager()->AllocateTempUploadBuffer(tempBuffer, totalBytes);

        // Update texture data from upload buffer to gpu buffer.
        pCommandList->CopyTextureBuffer(texture->GetTextureBuffer()->GetResource().Get(),
            tempBuffer->ResourceLocation.Resource.Get(), 0, i, 1, &textureData);
    }

    pCommandList->AddTransitionResourceBarriers(texture->GetTextureBuffer()->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    pCommandList->FlushResourceBarriers();

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
    blas[index].pScratchResource = std::make_shared<D3D12UnorderedAccessBuffer>(resourceDesc, viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        blas[index].pScratchResource.get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        L"ScratchResource");

    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
        bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    blas[index].pBottomLevelAccelerationStructure = std::make_shared<D3D12UnorderedAccessBuffer>(resourceDesc, viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        blas[index].pBottomLevelAccelerationStructure.get(),
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
    pInstanceDescBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateTempUploadBuffer(pInstanceDescBuffer, instanceDescsSize);

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
    topLevelInputs.InstanceDescs = pInstanceDescBuffer->ResourceLocation.Resource->GetGPUVirtualAddress();

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
    pDevice->GetDXRDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
    ThrowIfFalse(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0);

    D3D12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
        topLevelPrebuildInfo.ScratchDataSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
    tlas[index].pScratchResource = std::make_shared<D3D12UnorderedAccessBuffer>(resourceDesc, viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        tlas[index].pScratchResource.get(),
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        L"ScratchResource");

    resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(
        topLevelPrebuildInfo.ScratchDataSizeInBytes,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    tlas[index].pTopLevelAccelerationStructure = std::make_shared<D3D12UnorderedAccessBuffer>(resourceDesc, viewDesc);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(
        tlas[index].pTopLevelAccelerationStructure.get(),
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

    pCommandList->AddTransitionResourceBarriers(pFrustumCullingData->GetResource().Get(),
        pFrustumCullingData->GetResourceState(), D3D12_RESOURCE_STATE_COPY_DEST);
    pCommandList->FlushResourceBarriers();
    pCommandList->CopyResource(
        pFrustumCullingData->GetResource().Get(),
        pUploadBuffer->ResourceLocation.Resource.Get());
    pCommandList->AddTransitionResourceBarriers(pFrustumCullingData->GetResource().Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, pFrustumCullingData->GetResourceState());
    pCommandList->FlushResourceBarriers();
}
