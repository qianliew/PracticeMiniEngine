#include "stdafx.h"
#include "SceneManager.h"

SceneManager::SceneManager(shared_ptr<D3D12Device>& device) :
    pDevice(device),
    objectID(0)
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

void SceneManager::LoadScene(D3D12CommandList*& pCommandList)
{
    D3D12Model* model = new D3D12Model(objectID++, L"cube.fbx", L"test.png");
    model->LoadModel(pFBXImporter);
    model->MoveAlongX(10.0f);
    model->MoveAlongZ(5.0f);

    AddObject(model);

    model = new D3D12Model(objectID++, L"cube.fbx", L"test.png");
    model->LoadModel(pFBXImporter);
    model->MoveAlongY(2.0f);

    AddObject(model);

    pFullScreenMesh = new D3D12Model(objectID++, L"plane.fbx", nullptr);
    pFullScreenMesh->LoadModel(pFBXImporter);
    pFullScreenMesh->MoveAlongX(10.0f);
    pFullScreenMesh->MoveAlongZ(5.0f);

    // Create the global constant buffer.
    pDevice->GetBufferManager()->AllocateGlobalConstantBuffer();
    pDevice->GetBufferManager()->GetGlobalConstantBuffer()->CreateView(pDevice->GetDevice(),
        pDevice->GetDescriptorHeapManager()->GetHandle(CONSTANT_BUFFER_VIEW_GLOBAL, 0));

    for (UINT i = 0; i < pObjects.size(); i++)
    {
        D3D12Model* object = pObjects[i];
        LoadObjectVertexBufferAndIndexBuffer(pCommandList, object);
    }
    LoadObjectVertexBufferAndIndexBuffer(pCommandList, pFullScreenMesh);

    pCommandList->FlushResourceBarriers();
}

void SceneManager::UnloadScene()
{
    objectID = 0;

    for (auto it = pObjects.begin(); it != pObjects.end(); it++)
    {
        delete* it;
        *it = nullptr;
    }
    pObjects.clear();
}

void SceneManager::CreateCamera(UINT width, UINT height)
{
    pCamera = new D3D12Camera(0, static_cast<FLOAT>(width), static_cast<FLOAT>(height));
    pCamera->SetViewport(static_cast<FLOAT>(width), static_cast<FLOAT>(height));
    pCamera->SetScissorRect(static_cast<LONG>(width), static_cast<LONG>(height));
}

void SceneManager::AddObject(D3D12Model* object)
{
	pObjects.push_back(object);
}

// Create buffers relating to the material and their view.
void SceneManager::CreateAndBindObjectBuffer(D3D12CommandList*& pCommandList)
{
    for (UINT i = 0; i < pObjects.size(); i++)
    {
        D3D12Model* model = pObjects[i];

        // Create the texture buffer.
        D3D12UploadBuffer* tempBuffer = new D3D12UploadBuffer();
        pDevice->GetBufferManager()->AllocateUploadBuffer(tempBuffer, UploadBufferType::Texture);
        pDevice->GetBufferManager()->AllocateDefaultBuffer(model->GetTexture()->TextureBuffer);
        model->GetTexture()->TextureBuffer->CreateView(pDevice->GetDevice(),
            pDevice->GetDescriptorHeapManager()->GetHandle(SHADER_RESOURCE_VIEW, 0));

        // Init texture data.
        UINT64 rowSizeInBytes, totalBytes;
        pDevice->GetDevice()->GetCopyableFootprints(&model->GetTexture()->TextureBuffer->GetResourceDesc(),
            0, 1, 0, nullptr, nullptr, &rowSizeInBytes, &totalBytes);
        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = model->GetTexture()->GetTextureData();
        textureData.RowPitch = rowSizeInBytes;
        textureData.SlicePitch = totalBytes;

        // Update texture data from upload buffer to gpu buffer.
        pCommandList->CopyTextureBuffer(model->GetTexture()->TextureBuffer->GetResource(),
            tempBuffer->ResourceLocation.Resource.Get(), 0, 0, 1, &textureData);

        model->GetTexture()->CreateSampler();
        pDevice->GetDescriptorHeapManager()->GetSamplerHandle(model->GetTexture()->TextureSampler.get(), 0);
        pDevice->GetDevice()->CreateSampler(&model->GetTexture()->TextureSampler->SamplerDesc,
            model->GetTexture()->TextureSampler->CPUHandle);

        pCommandList->AddTransitionResourceBarriers(model->GetTexture()->TextureBuffer->GetResource(),
            D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    pCommandList->FlushResourceBarriers();
}

void SceneManager::DrawObjects(D3D12CommandList*& pCommandList)
{
    for (UINT i = 0; i < pObjects.size(); i++)
    {
        D3D12Model* model = pObjects[i];

        pCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pDevice->GetDescriptorHeapManager()->SetCBVs(pCommandList->GetCommandList(),
            CONSTANT_BUFFER_VIEW_PEROBJECT, pObjects[i]->GetObjectID());
        pCommandList->SetVertexBuffers(0, 1, &model->GetMesh()->VertexBuffer->VertexBufferView);
        pCommandList->SetIndexBuffer(&model->GetMesh()->IndexBuffer->IndexBufferView);
        pCommandList->DrawIndexedInstanced(model->GetMesh()->GetIndicesNum());
    }
}

void SceneManager::DrawFullScreenMesh(D3D12CommandList*& pCommandList)
{
    pCommandList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pDevice->GetDescriptorHeapManager()->SetCBVs(pCommandList->GetCommandList(),
        CONSTANT_BUFFER_VIEW_PEROBJECT, pFullScreenMesh->GetObjectID());
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

void SceneManager::LoadObjectVertexBufferAndIndexBuffer(D3D12CommandList*& pCommandList, D3D12Model* object)
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
}
