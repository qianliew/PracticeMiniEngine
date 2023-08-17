#pragma once

using namespace DirectX;
using std::unique_ptr;
using std::shared_ptr;

#define CAMERA_MOVING_SPEED 2.0f
#define CAMERA_ROTATING_SPEED XM_PI / 36.0f

class Camera
{
private:
    unique_ptr<XMVECTOR> m_cameraPosition;
    unique_ptr<XMVECTOR> m_fowardDirction;
    unique_ptr<XMVECTOR> m_upDirction;

    FLOAT fov;
    FLOAT ratio;
    FLOAT nearZ;
    FLOAT farZ;

public:
    Camera();

    ~Camera();

    XMMATRIX GetMVPMatrix();

    void ResetCamera();
    void MoveCameraAlongZ(FLOAT direction);
    void MoveCameraAlongX(FLOAT direction);
    void RotateCameraAlongY(FLOAT direction);
    void RotateCameraAlongX(FLOAT direction);
};