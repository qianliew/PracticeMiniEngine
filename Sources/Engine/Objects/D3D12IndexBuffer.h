#pragma once

class D3D12IndexBuffer : public D3D12Resource
{
private:
    D3D12_INDEX_BUFFER_VIEW indexBufferView;

public:
    D3D12IndexBuffer();
    ~D3D12IndexBuffer();

    virtual void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
    void SetIndexBufferView(const D3D12_INDEX_BUFFER_VIEW& view);

    inline const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return indexBufferView; }
};
