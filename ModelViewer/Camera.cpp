#include "stdafx.h"
#include "Camera.h"

Camera::Camera()
{
    m_cameraPosition = std::make_unique<XMVECTOR>();
    m_fowardDirction = std::make_unique<XMVECTOR>();
    m_upDirction = std::make_unique<XMVECTOR>();
    ResetCamera();
}

Camera::~Camera()
{
    m_cameraPosition.release();
    m_fowardDirction.release();
    m_upDirction.release();
}

XMMATRIX Camera::GetMVPMatrix()
{
    // Get the view matrix.
    XMMATRIX view = XMMatrixLookAtRH(*m_cameraPosition, *m_cameraPosition + *m_fowardDirction, *m_upDirction);

    // Get the proj matrix.
    XMMATRIX proj = XMMatrixPerspectiveFovRH(fov, ratio, nearZ, farZ);

    return view * proj;
}

void Camera::ResetCamera()
{
    *m_cameraPosition = XMVectorSet(0, 0, -10, 1);
    *m_fowardDirction = XMVectorSet(0, 0, 1, 0);
    *m_upDirction = XMVectorSet(0, 1, 0, 0);
    fov = XM_PI / 3.0f;
    ratio = 16.0f / 9.0f;
    nearZ = 0.03f;
    farZ = 100.0f;
}

void Camera::MoveCameraAlongZ(FLOAT direction)
{
    *m_cameraPosition += *m_fowardDirction * CAMERA_MOVING_SPEED * direction;
}

void Camera::MoveCameraAlongX(FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(*m_upDirction, *m_fowardDirction);

    *m_cameraPosition += vec * CAMERA_MOVING_SPEED * direction;
}

void Camera::RotateCameraAlongY(FLOAT direction)
{
    XMMATRIX trans = XMMatrixRotationNormal(g_XMIdentityR1, CAMERA_ROTATING_SPEED * direction);
    *m_fowardDirction = XMPlaneNormalize(XMVector3Transform(*m_fowardDirction, trans));
    *m_upDirction = XMPlaneNormalize(XMVector3Transform(*m_upDirction, trans));
}

void Camera::RotateCameraAlongX(FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(*m_upDirction, *m_fowardDirction);

    XMMATRIX trans = XMMatrixRotationNormal(vec, CAMERA_ROTATING_SPEED * direction);
    *m_fowardDirction = XMPlaneNormalize(XMVector3Transform(*m_fowardDirction, trans));
    *m_upDirction = XMPlaneNormalize(XMVector3Transform(*m_upDirction, trans));
}