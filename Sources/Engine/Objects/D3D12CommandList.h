#pragma once

#define MAX_RESOURCE_BARRIER 16

class D3D12CommandList
{
private:
    ComPtr<ID3D12GraphicsCommandList> pCommandList;
    ComPtr<ID3D12GraphicsCommandList4> pDXRCommandList;
    std::shared_ptr<D3D12Device> pDevice;

    D3D12_RESOURCE_BARRIER resourceBarriers[MAX_RESOURCE_BARRIER];
    UINT barrierIndex;

public:
    D3D12CommandList(std::shared_ptr<D3D12Device>&);
    ~D3D12CommandList();

    void ExecuteCommandList();

    inline ComPtr<ID3D12GraphicsCommandList>& GetCommandList() { return pCommandList; }
    inline ComPtr<ID3D12GraphicsCommandList4>& GetDXRCommandList() { return pDXRCommandList; }

    inline void Reset(ComPtr<ID3D12CommandAllocator>& commandAllocator)
    {
        ThrowIfFailed(pCommandList->Reset(commandAllocator.Get(), nullptr));
    }

    inline void SetPipelineState(ID3D12PipelineState* pipelineState)
    {
        pCommandList->SetPipelineState(pipelineState);
    }

    inline void SetRootSignature(ComPtr<ID3D12RootSignature>& rootSignature)
    {
        pCommandList->SetGraphicsRootSignature(rootSignature.Get());
    }

    inline void SetComputeRootSignature(ComPtr<ID3D12RootSignature>& rootSignature)
    {
        pCommandList->SetComputeRootSignature(rootSignature.Get());
    }

    inline void SetRootConstantBufferView(UINT index, D3D12_GPU_VIRTUAL_ADDRESS location)
    {
        pCommandList->SetGraphicsRootConstantBufferView(index, location);
    }

    inline void SetComputeRootConstantBufferView(UINT index, D3D12_GPU_VIRTUAL_ADDRESS location)
    {
        pCommandList->SetComputeRootConstantBufferView(index, location);
    }

    inline void SetComputeRootShaderResourceView(UINT index, D3D12_GPU_VIRTUAL_ADDRESS location)
    {
        pCommandList->SetComputeRootShaderResourceView(index, location);
    }

    inline void SetViewports(const D3D12_VIEWPORT* pViewports, UINT NumViewports = 1)
    {
        pCommandList->RSSetViewports(NumViewports, pViewports);
    }

    inline void SetScissorRects(const D3D12_RECT* pViewports, UINT NumViewports = 1)
    {
        pCommandList->RSSetScissorRects(NumViewports, pViewports);
    }

    inline void SetRenderTargets(UINT NumRenderTargetDescriptors,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
    {
        pCommandList->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, TRUE, pDepthStencilDescriptor);
    }

    inline void ClearColor(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT ColorRGBA[4])
    {
        pCommandList->ClearRenderTargetView(RenderTargetView, ColorRGBA, 0, nullptr);
    }

    inline void ClearDepth(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView)
    {
        pCommandList->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    inline void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
    {
        pCommandList->IASetPrimitiveTopology(PrimitiveTopology);
    }

    inline void SetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews)
    {
        pCommandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
    }

    inline void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView)
    {
        pCommandList->IASetIndexBuffer(pView);
    }

    inline void DrawIndexedInstanced(UINT IndexCountPerInstance)
    {
        pCommandList->DrawIndexedInstanced(IndexCountPerInstance, 1, 0, 0, 0);
    }

    inline void CopyBufferRegion(ID3D12Resource* pDstBuffer, ID3D12Resource* pSrcBuffer,
        UINT64 NumBytes, UINT64 DstOffset = 0, UINT64 SrcOffset = 0)
    {
        pCommandList->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);
    }

    inline void CopyTextureBuffer(ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate,
        UINT64 IntermediateOffset, UINT FirstSubresource, UINT NumSubresources, const D3D12_SUBRESOURCE_DATA* pSrcData)
    {
        UpdateSubresources(pCommandList.Get(), pDestinationResource, pIntermediate,
            IntermediateOffset, FirstSubresource, NumSubresources, pSrcData);
    }

    inline void CopyResource(ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource)
    {
        pCommandList->CopyResource(pDstResource, pSrcResource);
    }

    inline void CopyTexture(D3D12_TEXTURE_COPY_LOCATION* pDstResource, D3D12_TEXTURE_COPY_LOCATION* pSrcResource)
    {
        pCommandList->CopyTextureRegion(pDstResource, 0, 0, 0, pSrcResource, nullptr);
    }

    inline void AddTransitionResourceBarriers(ID3D12Resource* pResource,
        D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
    {
        resourceBarriers[barrierIndex].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        resourceBarriers[barrierIndex].Transition.pResource = pResource;
        resourceBarriers[barrierIndex].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        resourceBarriers[barrierIndex].Transition.StateBefore = stateBefore;
        resourceBarriers[barrierIndex++].Transition.StateAfter = stateAfter;
    }

    inline void FlushResourceBarriers()
    {
        pCommandList->ResourceBarrier(barrierIndex, &resourceBarriers[0]);
        barrierIndex = 0;
    }
};
