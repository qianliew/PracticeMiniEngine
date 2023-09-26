//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "DXSample.h"
#include "FBXImporter.h"
#include "Camera.h"
#include "Mesh.h"
#include "Texture.h"

using namespace DirectX;

// Note that while ComPtr is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using Microsoft::WRL::ComPtr;
using std::unique_ptr;
using std::shared_ptr;

class ModelViewer : public DXSample
{
public:
    ModelViewer(UINT width, UINT height, std::wstring name);

    virtual void OnInit();
    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnDestroy();
    void OnKeyDown(UINT8 /*key*/) override;
    void OnKeyUp(UINT8 /*key*/) override;

private:
    static const UINT FrameCount = 2;
    UINT m_width;
    UINT m_height;

    struct Constant
    {
        XMFLOAT4X4 ObjectToWorldMatrix;
        XMFLOAT4 a;
    };

    // Pipeline objects.
    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
    ComPtr<IDXGISwapChain3> m_swapChain;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
    ComPtr<ID3D12Resource> m_depthStencils[FrameCount];
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    ComPtr<ID3D12DescriptorHeap> m_cbvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_rtvDescriptorSize;
    UINT m_dsvDescriptorSize;

    unique_ptr<D3D12BufferManager> m_allocator;
    unique_ptr<D3D12DescriptorHeapManager> m_descriptorHeapManager;

    // App resources.
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

    unique_ptr<D3D12UploadBuffer> m_indexBuffer;
    unique_ptr<D3D12UploadBuffer> m_constantBuffer;
    unique_ptr<D3D12DefaultBuffer> m_indexStaticBuffer;

    // Synchronization objects.
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;

    // Scene objects
    shared_ptr<Constant> m_constant;
    shared_ptr<Camera> m_camera;
    shared_ptr<Mesh> m_mesh;
    shared_ptr<Texture> m_texture;
    unique_ptr<FBXImporter> m_fbxImporter;

    void LoadPipeline();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();
};
