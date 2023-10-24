#pragma once

#define MAX_RESOURCE_BARRIER 16

class D3D12CommandList
{
private:
    ComPtr<ID3D12GraphicsCommandList> commandList;

    D3D12_RESOURCE_BARRIER resourceBarriers[MAX_RESOURCE_BARRIER];
    UINT barrierIndex;

public:
    D3D12CommandList(ComPtr<ID3D12Device>&, ComPtr<ID3D12CommandAllocator>&);
    ~D3D12CommandList();

    void SetPipelineState(ComPtr<ID3D12CommandAllocator>&, ComPtr<ID3D12PipelineState>&);
    void SetRootSignature(ComPtr<ID3D12RootSignature>&);
    void SetViewports(const D3D12_VIEWPORT* pViewports, UINT NumViewports = 1);
    void SetScissorRects(const D3D12_RECT* pViewports, UINT NumViewports = 1);
    void SetRenderTargets(UINT NumRenderTargetDescriptors,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
        const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor);
    void ClearColor(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT ColorRGBA[4]);
    void ClearDepth(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView);
    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology);
    void SetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
    void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView);
    void DrawIndexedInstanced(UINT IndexCountPerInstance);
    void CopyBufferRegion(ID3D12Resource* pDstBuffer, ID3D12Resource* pSrcBuffer,
        UINT64 NumBytes, UINT64 DstOffset = 0, UINT64 SrcOffset = 0);
    void CopyTextureBuffer(ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate,
        UINT64 IntermediateOffset, UINT FirstSubresource, UINT NumSubresources, const D3D12_SUBRESOURCE_DATA* pSrcData);
    void AddTransitionResourceBarriers(ID3D12Resource* pResource,
        D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
    void FlushResourceBarriers();

    ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList; }
};

inline void D3D12CommandList::SetPipelineState(ComPtr<ID3D12CommandAllocator>& commandAllocator, ComPtr<ID3D12PipelineState>&pipelineState)
{
    ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState.Get()));
}

inline void D3D12CommandList::SetRootSignature(ComPtr<ID3D12RootSignature>& rootSignature)
{
    commandList->SetGraphicsRootSignature(rootSignature.Get());
}

inline void D3D12CommandList::SetViewports(const D3D12_VIEWPORT* pViewports, UINT NumViewports)
{
    commandList->RSSetViewports(NumViewports, pViewports);
}

inline void D3D12CommandList::SetScissorRects(const D3D12_RECT* pViewports, UINT NumViewports)
{
    commandList->RSSetScissorRects(NumViewports, pViewports);
}

inline void D3D12CommandList::SetRenderTargets(UINT NumRenderTargetDescriptors,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescriptors,
    const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor)
{
    commandList->OMSetRenderTargets(NumRenderTargetDescriptors, pRenderTargetDescriptors, FALSE, pDepthStencilDescriptor);
}

inline void D3D12CommandList::ClearColor(D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetView, const FLOAT ColorRGBA[4])
{
    commandList->ClearRenderTargetView(RenderTargetView, ColorRGBA, 0, nullptr);
}

inline void D3D12CommandList::ClearDepth(D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView)
{
    commandList->ClearDepthStencilView(DepthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

inline void D3D12CommandList::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
{
    commandList->IASetPrimitiveTopology(PrimitiveTopology);
}

inline void D3D12CommandList::SetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews)
{
    commandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
}

inline void D3D12CommandList::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView)
{
    commandList->IASetIndexBuffer(pView);
}

inline void D3D12CommandList::DrawIndexedInstanced(UINT IndexCountPerInstance)
{
    commandList->DrawIndexedInstanced(IndexCountPerInstance, 1, 0, 0, 0);
}

inline void D3D12CommandList::CopyBufferRegion(ID3D12Resource* pDstBuffer, ID3D12Resource* pSrcBuffer,
    UINT64 NumBytes, UINT64 DstOffset, UINT64 SrcOffset)
{
    commandList->CopyBufferRegion(pDstBuffer, DstOffset, pSrcBuffer, SrcOffset, NumBytes);
}

inline void D3D12CommandList::CopyTextureBuffer(ID3D12Resource* pDestinationResource, ID3D12Resource* pIntermediate,
    UINT64 IntermediateOffset, UINT FirstSubresource, UINT NumSubresources, const D3D12_SUBRESOURCE_DATA* pSrcData)
{
    UpdateSubresources(commandList.Get(), pDestinationResource, pIntermediate,
        IntermediateOffset, FirstSubresource, NumSubresources, pSrcData);
}

inline void D3D12CommandList::AddTransitionResourceBarriers(ID3D12Resource* pResource,
    D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
    resourceBarriers[barrierIndex].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    resourceBarriers[barrierIndex].Transition.pResource = pResource;
    resourceBarriers[barrierIndex].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    resourceBarriers[barrierIndex].Transition.StateBefore = stateBefore;
    resourceBarriers[barrierIndex++].Transition.StateAfter = stateAfter;
}

inline void D3D12CommandList::FlushResourceBarriers()
{
    commandList->ResourceBarrier(barrierIndex, &resourceBarriers[0]);
    barrierIndex = 0;
}
