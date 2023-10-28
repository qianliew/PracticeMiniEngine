#pragma once

class D3D12IndexBuffer : public D3D12Resource
{
private:

public:
    D3D12IndexBuffer(const D3D12_RESOURCE_DESC&);
    ~D3D12IndexBuffer();

    void CreateView(const ComPtr<ID3D12Device>& device, const D3D12_CPU_DESCRIPTOR_HANDLE& handle) override;
    void CreateView();

    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
};
