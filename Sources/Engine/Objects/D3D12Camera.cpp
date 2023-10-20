#include "stdafx.h"
#include "D3D12Camera.h"

D3D12Camera::D3D12Camera(FLOAT width, FLOAT height) :
    width(width),
    height(height),
    fov(CAMERA_DEFAULT_FOV),
    aspectRatio(width / height),
    nearZ(CAMERA_DEFAULT_NEAR_Z),
    farZ(CAMERA_DEFAULT_FAR_Z),
    cameraPosition(DefaultCameraPosition),
    forwardDirction(DefaultForwardDirction),
    upDirction(DefaultUpDirction)
{
    pViewport = new CD3DX12_VIEWPORT(0.0f, 0.0f, width, height);
    pScissorRect = new CD3DX12_RECT(0.0f, 0.0f, static_cast<LONG>(width), static_cast<LONG>(height));
}

D3D12Camera::~D3D12Camera()
{
    delete pViewport;
    delete pScissorRect;
}

void D3D12Camera::ResetCamera()
{
    cameraPosition = DefaultCameraPosition;
    forwardDirction = DefaultForwardDirction;
    upDirction = DefaultUpDirction;

    fov = CAMERA_DEFAULT_FOV;
    // aspectRatio = CAMERA_DEFAULT_ASPECT_RATIO;
    nearZ = CAMERA_DEFAULT_NEAR_Z;
    farZ = CAMERA_DEFAULT_FAR_Z;

    SetViewport(width, height);
    SetScissorRect(static_cast<LONG>(width), static_cast<LONG>(height));
}

void D3D12Camera::SetViewport(const FLOAT width, const FLOAT height)
{
    pViewport->Width = width;
    pViewport->Height = height;
}

void D3D12Camera::SetScissorRect(const LONG width, const LONG height)
{
    pScissorRect->right = width;
    pScissorRect->bottom = height;
}

void D3D12Camera::MoveCameraAlongZ(const FLOAT direction)
{
    cameraPosition += forwardDirction * CAMERA_MOVING_SPEED * direction;
}

void D3D12Camera::MoveCameraAlongX(const FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(upDirction, forwardDirction);

    cameraPosition += vec * CAMERA_MOVING_SPEED * direction;
}

void D3D12Camera::RotateCameraAlongY(const FLOAT direction)
{
    XMMATRIX trans = XMMatrixRotationNormal(g_XMIdentityR1, CAMERA_ROTATING_SPEED * direction);
    forwardDirction = XMPlaneNormalize(XMVector3Transform(forwardDirction, trans));
    upDirction = XMPlaneNormalize(XMVector3Transform(upDirction, trans));
}

void D3D12Camera::RotateCameraAlongX(const FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(upDirction, forwardDirction);

    XMMATRIX trans = XMMatrixRotationNormal(vec, CAMERA_ROTATING_SPEED * direction);
    forwardDirction = XMPlaneNormalize(XMVector3Transform(forwardDirction, trans));
    upDirction = XMPlaneNormalize(XMVector3Transform(upDirction, trans));
}

const XMMATRIX D3D12Camera::GetMVPMatrix()
{
    // Get the view matrix.
    XMMATRIX view = XMMatrixLookAtRH(cameraPosition, cameraPosition + forwardDirction, upDirction);

    // Get the proj matrix.
    XMMATRIX proj = XMMatrixPerspectiveFovRH(fov, aspectRatio, nearZ, farZ);

    return view * proj;
}
