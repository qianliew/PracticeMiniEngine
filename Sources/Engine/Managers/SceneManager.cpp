#include "stdafx.h"
#include "SceneManager.h"

SceneManager::SceneManager(shared_ptr<D3D12Device>& device) :
    pDevice(device),
    objectID(0),
    textureID(0)
{

}

SceneManager::~SceneManager()
{
    UnloadScene();
    delete pCamera;
}

void SceneManager::InitFBXImporter()
{
    pFBXImporter.release();
    pFBXImporter = std::make_unique<FBXImporter>();
    pFBXImporter->InitializeSdkObjects();
}

void SceneManager::ParseScene(D3D12CommandList*& pCommandList)
{
    LPCWSTR sceneName = L"scene";
    std::wifstream inFile(GetAssetPath(sceneName));

    // Parse textures from the scene file.
    UINT numTextures = 0;
    inFile >> numTextures;

    for (int i = 0; i < numTextures; i++)
    {
        WCHAR textureName[50];
        inFile >> textureName;

        D3D12Texture* texture = new D3D12Texture(textureID++);
        texture->LoadTexture(GetAssetPath(textureName).c_str());
        texture->CreateTexture(D3D12TextureType::ShaderResource);
        LoadTextureBufferAndSampler(pCommandList, texture);

        pTextures[textureName] = texture;
    }

    // Parse FBX from the scene file.
    UINT numModels = 0;
    inFile >> numModels;

    for (int i = 0; i < numTextures; i++)
    {
        WCHAR name[50];
        inFile >> name;

        Model* model = new Model(objectID++, name);
        model->LoadModel(pFBXImporter);
        model->AddTexture(i);

        AddObject(model);
        LoadObjectVertexBufferAndIndexBuffer(pCommandList, model);
    }

    inFile.close();
}

void SceneManager::LoadScene(D3D12CommandList*& pCommandList)
{
    // Parse the scene file.
    ParseScene(pCommandList);

    // Create static data.
    pFullScreenMesh = new Model(objectID++, L"plane.fbx");
    pFullScreenMesh->LoadModel(pFBXImporter);
    pFullScreenMesh->MoveAlongX(10.0f);
    pFullScreenMesh->MoveAlongZ(5.0f);

    // Create the global constant buffer.
    pDevice->GetBufferManager()->AllocateGlobalConstantBuffer();
    pDevice->GetBufferManager()->GetGlobalConstantBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(CONSTANT_BUFFER_VIEW_GLOBAL, 0));

    LoadObjectVertexBufferAndIndexBuffer(pCommandList, pFullScreenMesh);
}

void SceneManager::UnloadScene()
{
    objectID = 0;
    textureID = 0;

    for (auto it = pObjects.begin(); it != pObjects.end(); it++)
    {
        delete* it;
    }
    pObjects.clear();

    for (auto it = pTextures.begin(); it != pTextures.end(); it++)
    {
        delete it->second;
    }
    pTextures.clear();
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

void SceneManager::DrawObjects(D3D12CommandList*& pCommandList)
{
    for (UINT i = 0; i < pObjects.size(); i++)
    {
        Model* model = pObjects[i];
        UINT id = pObjects[i]->GetObjectID();

        // Set the per object views.
        pCommandList->SetRootConstantBufferView(CONSTANT_BUFFER_VIEW_PEROBJECT,
            pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(id)->GetResource()->GetGPUVirtualAddress());

        // Set the material relating views.
        pDevice->GetDescriptorHeapManager()->SetSRVs(pCommandList->GetCommandList(),
            model->GetMaterial()->GetTextureIDAt(0));
        pDevice->GetDescriptorHeapManager()->SetSamplers(pCommandList->GetCommandList(),
            model->GetMaterial()->GetTextureIDAt(0));

        // Set buffers and draw the instance.
        pCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pCommandList->SetVertexBuffers(0, 1, &model->GetMesh()->VertexBuffer->VertexBufferView);
        pCommandList->SetIndexBuffer(&model->GetMesh()->IndexBuffer->IndexBufferView);
        pCommandList->DrawIndexedInstanced(model->GetMesh()->GetIndicesNum());
    }
}

void SceneManager::DrawFullScreenMesh(D3D12CommandList*& pCommandList)
{
    pCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pCommandList->SetVertexBuffers(0, 1, &pFullScreenMesh->GetMesh()->VertexBuffer->VertexBufferView);
    pCommandList->SetIndexBuffer(&pFullScreenMesh->GetMesh()->IndexBuffer->IndexBufferView);
    pCommandList->DrawIndexedInstanced(pFullScreenMesh->GetMesh()->GetIndicesNum());
}

void SceneManager::UpdateTransforms()
{
    for (UINT i = 0; i < pObjects.size(); i++)
    {
        pObjects[i]->SetObjectToWorldMatrix();
        TransformConstant transformConstant = pObjects[i]->GetTransformConstant();
        pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(pObjects[i]->GetObjectID())
            ->CopyData(&transformConstant, sizeof(TransformConstant));
    }
}

void SceneManager::UpdateCamera()
{
    CameraConstant cameraConstant = pCamera->GetCameraConstant();
    XMStoreFloat4x4(&cameraConstant.WorldToProjectionMatrix, pCamera->GetVPMatrix());
    pDevice->GetBufferManager()->GetGlobalConstantBuffer()->CopyData(&cameraConstant, sizeof(CameraConstant));
}

// Helper functions.
void SceneManager::LoadObjectVertexBufferAndIndexBuffer(D3D12CommandList*& pCommandList, Model* object)
{
    // Create the perObject constant buffer and its view.
    UINT id = object->GetObjectID();
    pDevice->GetBufferManager()->AllocatePerObjectConstantBuffers(id);
    pDevice->GetBufferManager()->GetPerObjectConstantBufferAtIndex(id)->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(CONSTANT_BUFFER_VIEW_PEROBJECT, id));

    // Create the vertex buffer and index buffer and their view.
    D3D12UploadBuffer* tempVertexBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateUploadBuffer(tempVertexBuffer, UploadBufferType::Vertex);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(object->GetMesh()->VertexBuffer.get());
    tempVertexBuffer->CopyData(object->GetMesh()->GetVerticesData(), object->GetMesh()->GetVerticesSize());

    D3D12UploadBuffer* tempIndexBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateUploadBuffer(tempIndexBuffer, UploadBufferType::Index);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(object->GetMesh()->IndexBuffer.get());
    tempIndexBuffer->CopyData(object->GetMesh()->GetIndicesData(), object->GetMesh()->GetIndicesSize());

    object->GetMesh()->CreateView();
    pCommandList->CopyBufferRegion(object->GetMesh()->VertexBuffer->GetResource(),
        tempVertexBuffer->ResourceLocation.Resource.Get(),
        object->GetMesh()->GetVerticesSize());
    pCommandList->CopyBufferRegion(object->GetMesh()->IndexBuffer->GetResource(),
        tempIndexBuffer->ResourceLocation.Resource.Get(),
        object->GetMesh()->GetIndicesSize());

    // Setup transition barriers.
    pCommandList->AddTransitionResourceBarriers(object->GetMesh()->VertexBuffer->GetResource(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    pCommandList->AddTransitionResourceBarriers(object->GetMesh()->IndexBuffer->GetResource(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    pCommandList->FlushResourceBarriers();
}

void SceneManager::LoadTextureBufferAndSampler(D3D12CommandList*& pCommandList, D3D12Texture* texture)
{
    UINT id = texture->GetTextureID();

    // Create the texture buffer.
    D3D12UploadBuffer* tempBuffer = new D3D12UploadBuffer();
    pDevice->GetBufferManager()->AllocateUploadBuffer(tempBuffer, UploadBufferType::Texture);
    pDevice->GetBufferManager()->AllocateDefaultBuffer(texture->TextureBuffer);
    texture->TextureBuffer->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(SHADER_RESOURCE_VIEW, id));

    // Init texture data.
    UINT64 rowSizeInBytes, totalBytes;
    pDevice->GetDevice()->GetCopyableFootprints(&texture->TextureBuffer->GetResourceDesc(),
        0, 1, 0, nullptr, nullptr, &rowSizeInBytes, &totalBytes);
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = texture->GetTextureData();
    textureData.RowPitch = rowSizeInBytes;
    textureData.SlicePitch = totalBytes;

    // Update texture data from upload buffer to gpu buffer.
    pCommandList->CopyTextureBuffer(texture->TextureBuffer->GetResource(),
        tempBuffer->ResourceLocation.Resource.Get(), 0, 0, 1, &textureData);

    // Create the sampler and its view.
    texture->CreateSampler();
    pDevice->GetDescriptorHeapManager()->GetSamplerHandle(texture->TextureSampler.get(), id);
    pDevice->GetDevice()->CreateSampler(&texture->TextureSampler->SamplerDesc,
        texture->TextureSampler->CPUHandle);

    pCommandList->AddTransitionResourceBarriers(texture->TextureBuffer->GetResource(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    pCommandList->FlushResourceBarriers();
}
