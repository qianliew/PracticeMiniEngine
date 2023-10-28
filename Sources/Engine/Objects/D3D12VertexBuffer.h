#pragma once

class D3D12VertexBuffer : public D3D12Resource
{
private:

public:
    D3D12VertexBuffer(const D3D12_RESOURCE_DESC&);
    ~D3D12VertexBuffer();

    void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
    void CreateView();

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
};
