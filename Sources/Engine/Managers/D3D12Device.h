#pragma once

class D3D12Device
{
private:
    // Adapter info.
    bool useWarpDevice;

    ComPtr<ID3D12Device> pDevice;
    ComPtr<IDXGIFactory4> pFactory;
    ComPtr<IDXGISwapChain3> pSwapChain;
    ComPtr<ID3D12CommandQueue> pCommandQueue;

    void GetHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);

public:
    D3D12Device();
    ~D3D12Device();

    void CreateDevice(DXGI_SWAP_CHAIN_DESC1&);

    ComPtr<ID3D12Device> GetDevice() const { return pDevice; }
    ComPtr<IDXGIFactory4> GetFactory() const { return pFactory; }
    ComPtr<IDXGISwapChain3> GetSwapChain() const { return pSwapChain; }
    ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return pCommandQueue; }
};
