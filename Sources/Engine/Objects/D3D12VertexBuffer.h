#pragma once

class D3D12VertexBuffer : public D3D12Resource
{
private:
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

public:
    D3D12VertexBuffer();
    ~D3D12VertexBuffer();

    virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
    void SetVertexBufferView(const D3D12_VERTEX_BUFFER_VIEW& view);

    inline const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView; }
};
