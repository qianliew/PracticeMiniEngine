#pragma once

using namespace DirectX;

#define CAMERA_DEFAULT_FOV XM_PI / 3.0f
#define CAMERA_DEFAULT_ASPECT_RATIO 16.0f / 9.0f
#define CAMERA_DEFAULT_NEAR_Z 0.03f
#define CAMERA_DEFAULT_FAR_Z 1000.0f

#define CAMERA_MOVING_SPEED 2.0f
#define CAMERA_ROTATING_SPEED XM_PI / 36.0f

class D3D12Camera
{
private:
    CD3DX12_VIEWPORT* pViewport;
    CD3DX12_RECT* pScissorRect;

    const XMVECTOR DefaultCameraPosition = XMVectorSet(0, 0, -50, 1);
    const XMVECTOR DefaultForwardDirction = XMVectorSet(0, 0, 1, 0);
    const XMVECTOR DefaultUpDirction = XMVectorSet(0, 1, 0, 0);

    XMVECTOR cameraPosition;
    XMVECTOR forwardDirction;
    XMVECTOR upDirction;

    FLOAT width;
    FLOAT height;
    FLOAT fov;
    FLOAT aspectRatio;
    FLOAT nearZ;
    FLOAT farZ;

public:
    D3D12Camera(FLOAT width, FLOAT height);

    ~D3D12Camera();

    void ResetCamera();
    void SetViewport(const FLOAT width, const FLOAT height);
    void SetScissorRect(const LONG width, const LONG height);
    void MoveCameraAlongZ(const FLOAT direction);
    void MoveCameraAlongX(const FLOAT direction);
    void RotateCameraAlongY(const FLOAT direction);
    void RotateCameraAlongX(const FLOAT direction);

    const D3D12_VIEWPORT* GetViewport() const { return pViewport; }
    const D3D12_RECT* GetScissorRect() const { return pScissorRect; }
    const XMMATRIX GetMVPMatrix();
};
