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

    inline void SetComputeRoot32BitConstant(UINT index, UINT num, const void* pSrcData, UINT offset = 0)
    {
        pCommandList->SetComputeRoot32BitConstants(index, num, pSrcData, offset);
    }

    inline void SetComputeRootConstantBufferView(UINT index, D3D12_GPU_VIRTUAL_ADDRESS location)
    {
        pCommandList->SetComputeRootConstantBufferView(index, location);
    }

    inline void SetComputeRootShaderResourceView(UINT index, D3D12_GPU_VIRTUAL_ADDRESS location)
    {
        pCommandList->SetComputeRootShaderResourceView(index, location);
    }

    inline void SetComputeRootUnorderedAccessView(UINT index, D3D12_GPU_VIRTUAL_ADDRESS location)
    {
        pCommandList->SetComputeRootUnorderedAccessView(index, location);
    }

    inline void SetViewports(const D3D12_VIEWPORT* pViewports, UINT NumViewports = 1)
    {
        pCommandList->RSSetViewports(NumViewports, pViewports);
    }

    inline void SetScissorRects(const D3D12_RECT* pViewports, UINT NumViewports = 1)
    {
        pCommandList->RSSetScissorRects(NumViewports, pViewports);
    }

    inline void SetRenderTargets(
        UINT NumRenderTargetDescriptors,
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

    inline void ExecuteIndirect(
        ID3D12CommandSignature* pCommandSignature,
        UINT MaxCommandCount,
        ID3D12Resource* pArgumentBuffer,
        UINT64 ArgumentBufferOffset,
        ID3D12Resource* pCountBuffer,
        UINT64 CountBufferOffset)
    {
        pCommandList->ExecuteIndirect(pCommandSignature, MaxCommandCount, pArgumentBuffer,
            ArgumentBufferOffset, pCountBuffer, CountBufferOffset);
    }

    inline void DispatchThreads(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ)
    {
        pCommandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
    }

    // All copy methods.

    void CopyBufferRegion(
        D3D12Resource* pDstResource,
        ID3D12Resource* pSrcResource,
        UINT64 NumBytes,
        UINT64 DstOffset = 0,
        UINT64 SrcOffset = 0);
    void CopyResource(
        ID3D12Resource* pDstResource,
        ID3D12Resource* pSrcResource);
    void CopyResource(
        D3D12Resource* pDstResource,
        ID3D12Resource* pSrcResource);
    void CopyResource(
        ID3D12Resource* pDstResource,
        D3D12Resource* pSrcResource);
    void CopyResource(
        D3D12Resource* pDstResource,
        D3D12Resource* pSrcResource);
    void CopyTextureBuffer(
        D3D12Resource* pDstResource,
        ID3D12Resource* pIntermediate,
        UINT64 IntermediateOffset,
        UINT FirstSubresource,
        UINT NumSubresources,
        const D3D12_SUBRESOURCE_DATA* pSrcData);
    void CopyTexture(
        D3D12_TEXTURE_COPY_LOCATION* pDstResource,
        D3D12_TEXTURE_COPY_LOCATION* pSrcResource);

    void AddTransitionResourceBarriers(
        ID3D12Resource* pResource,
        D3D12_RESOURCE_STATES stateBefore,
        D3D12_RESOURCE_STATES stateAfter);
    void FlushResourceBarriers();
};
