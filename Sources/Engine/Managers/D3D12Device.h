#pragma once
#include "D3D12DescriptorHeapManager.h"
#include "D3D12BufferManager.h"

class D3D12Device
{
protected:
    // Adapter info.
    bool useWarpDevice;

    ComPtr<ID3D12Device> pDevice;
    ComPtr<IDXGIFactory4> pFactory;
    ComPtr<ID3D12CommandQueue> pCommandQueue;
    ComPtr<ID3D12CommandAllocator> pCommandAllocator;

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12Device5> pDXRDevice;

    D3D12DescriptorHeapManager* pDescriptorHeapManager;
    D3D12BufferManager* pBufferManager;

    void GetHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);

public:
    D3D12Device();
    ~D3D12Device();

    void CreateDevice();
    void CreateDescriptorHeapManager();
    void CreateBufferManager();

    inline ComPtr<ID3D12Device> GetDevice() const { return pDevice; }
    inline ComPtr<ID3D12Device5> GetDXRDevice() const { return pDXRDevice; }
    inline ComPtr<IDXGIFactory4> GetFactory() const { return pFactory; }
    inline ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return pCommandQueue; }
    inline ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const { return pCommandAllocator; }

    inline D3D12DescriptorHeapManager* GetDescriptorHeapManager() const { return pDescriptorHeapManager; }
    inline D3D12BufferManager* GetBufferManager() const { return pBufferManager; }
};
