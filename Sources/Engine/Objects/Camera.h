#pragma once

#include "Transform.h"
#include "D3D12ConstantBuffer.h"

#define CAMERA_DEFAULT_FOV XM_PI / 3.0f
#define CAMERA_DEFAULT_ASPECT_RATIO 16.0f / 9.0f
#define CAMERA_DEFAULT_NEAR_Z 0.03f
#define CAMERA_DEFAULT_FAR_Z 100000.0f

using namespace DirectX;

class Camera : public Transform
{
private:
    const XMVECTOR DefaultCameraWorldPosition = XMVectorSet(0.0f, 20.0f, -50.0f, 1.0f);

    CD3DX12_VIEWPORT* pViewport;
    CD3DX12_RECT* pScissorRect;

    CameraConstant cameraConstant;

    FLOAT width;
    FLOAT height;
    FLOAT fov;
    FLOAT aspectRatio;
    FLOAT nearZ;
    FLOAT farZ;

public:
    Camera(UINT id, FLOAT width, FLOAT height);
    ~Camera();

    virtual void ResetTransform() override;

    void SetViewport(const FLOAT width, const FLOAT height);
    void SetScissorRect(const LONG width, const LONG height);
    void UpdateCameraConstant();
    const XMMATRIX GetVPMatrix();

    inline const FLOAT GetCameraWidth() const { return width; }
    inline const FLOAT GetCameraHeight() const { return height; }
    inline const D3D12_VIEWPORT* GetViewport() const { return pViewport; }
    inline const D3D12_RECT* GetScissorRect() const { return pScissorRect; }
    inline CameraConstant& GetCameraConstant() { return cameraConstant; }
};
