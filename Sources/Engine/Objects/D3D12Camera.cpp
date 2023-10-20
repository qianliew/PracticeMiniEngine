#include "stdafx.h"
#include "D3D12Camera.h"

D3D12Camera::D3D12Camera()
{
    m_cameraPosition = std::make_unique<XMVECTOR>();
    m_fowardDirction = std::make_unique<XMVECTOR>();
    m_upDirction = std::make_unique<XMVECTOR>();
    ResetCamera();
}

D3D12Camera::~D3D12Camera()
{
    m_cameraPosition.release();
    m_fowardDirction.release();
    m_upDirction.release();
}

XMMATRIX D3D12Camera::GetMVPMatrix()
{
    // Get the view matrix.
    XMMATRIX view = XMMatrixLookAtRH(*m_cameraPosition, *m_cameraPosition + *m_fowardDirction, *m_upDirction);

    // Get the proj matrix.
    XMMATRIX proj = XMMatrixPerspectiveFovRH(fov, ratio, nearZ, farZ);

    return view * proj;
}

void D3D12Camera::ResetCamera()
{
    *m_cameraPosition = XMVectorSet(0, 0, -50, 1);
    *m_fowardDirction = XMVectorSet(0, 0, 1, 0);
    *m_upDirction = XMVectorSet(0, 1, 0, 0);
    fov = XM_PI / 3.0f;
    ratio = 16.0f / 9.0f;
    nearZ = 0.03f;
    farZ = 1000.0f;
}

void D3D12Camera::MoveCameraAlongZ(FLOAT direction)
{
    *m_cameraPosition += *m_fowardDirction * CAMERA_MOVING_SPEED * direction;
}

void D3D12Camera::MoveCameraAlongX(FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(*m_upDirction, *m_fowardDirction);

    *m_cameraPosition += vec * CAMERA_MOVING_SPEED * direction;
}

void D3D12Camera::RotateCameraAlongY(FLOAT direction)
{
    XMMATRIX trans = XMMatrixRotationNormal(g_XMIdentityR1, CAMERA_ROTATING_SPEED * direction);
    *m_fowardDirction = XMPlaneNormalize(XMVector3Transform(*m_fowardDirction, trans));
    *m_upDirction = XMPlaneNormalize(XMVector3Transform(*m_upDirction, trans));
}

void D3D12Camera::RotateCameraAlongX(FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(*m_upDirction, *m_fowardDirction);

    XMMATRIX trans = XMMatrixRotationNormal(vec, CAMERA_ROTATING_SPEED * direction);
    *m_fowardDirction = XMPlaneNormalize(XMVector3Transform(*m_fowardDirction, trans));
    *m_upDirction = XMPlaneNormalize(XMVector3Transform(*m_upDirction, trans));
}