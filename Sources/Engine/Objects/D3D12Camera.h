#pragma once

#define CAMERA_DEFAULT_FOV XM_PI / 3.0f
#define CAMERA_DEFAULT_ASPECT_RATIO 16.0f / 9.0f
#define CAMERA_DEFAULT_NEAR_Z 0.03f
#define CAMERA_DEFAULT_FAR_Z 1000.0f

using namespace DirectX;

struct CameraConstant
{
    XMFLOAT4X4 WorldToProjectionMatrix;
};

class D3D12Camera : public Transform
{
private:
    CD3DX12_VIEWPORT* pViewport;
    CD3DX12_RECT* pScissorRect;

    CameraConstant cameraConstant;

    FLOAT width;
    FLOAT height;
    FLOAT fov;
    FLOAT aspectRatio;
    FLOAT nearZ;
    FLOAT farZ;

    shared_ptr<D3D12UploadBuffer> cameraConstantBuffer;
    D3D12_CONSTANT_BUFFER_VIEW_DESC cameraCBVDesc;

public:
    D3D12Camera(FLOAT width, FLOAT height);

    ~D3D12Camera();

    virtual void ResetTransform() override;

    void SetViewport(const FLOAT width, const FLOAT height);
    void SetScissorRect(const LONG width, const LONG height);

    const D3D12_VIEWPORT* GetViewport() const { return pViewport; }
    const D3D12_RECT* GetScissorRect() const { return pScissorRect; }
    CameraConstant& GetCameraConstant() { return cameraConstant; }

    const XMMATRIX GetVPMatrix();

    void CreateCameraView();
    const shared_ptr<D3D12UploadBuffer> GetCameraConstantBuffer() const { return cameraConstantBuffer; }
    const D3D12_CONSTANT_BUFFER_VIEW_DESC GetCameraCBVDesc() const { return cameraCBVDesc; }
};
