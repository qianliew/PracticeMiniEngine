#include "stdafx.h"
#include "Camera.h"
#include "ViewManager.h"

Camera::Camera(UINT id, FLOAT width, FLOAT height) :
    Transform(id),
    width(width),
    height(height),
    fov(CAMERA_DEFAULT_FOV),
    aspectRatio(width / height),
    nearZ(CAMERA_DEFAULT_NEAR_Z),
    farZ(CAMERA_DEFAULT_FAR_Z)
{
    pViewport = new CD3DX12_VIEWPORT(0.0f, 0.0f, width, height);
    pScissorRect = new CD3DX12_RECT(0.0f, 0.0f, static_cast<LONG>(width), static_cast<LONG>(height));
    worldPosition = DefaultCameraWorldPosition;
    cameraConstant.IdentityProjectionMatrix = XMMatrixIdentity();
}

Camera::~Camera()
{
    delete pViewport;
    delete pScissorRect;
}

void Camera::ResetTransform()
{
    worldPosition = DefaultCameraWorldPosition;
    forwardDirction = DefaultForwardDirction;
    upDirction = DefaultUpDirction;

    fov = CAMERA_DEFAULT_FOV;
    // aspectRatio = CAMERA_DEFAULT_ASPECT_RATIO;
    nearZ = CAMERA_DEFAULT_NEAR_Z;
    farZ = CAMERA_DEFAULT_FAR_Z;

    SetViewport(width, height);
    SetScissorRect(static_cast<LONG>(width), static_cast<LONG>(height));
}

void Camera::SetViewport(const FLOAT width, const FLOAT height)
{
    pViewport->Width = width;
    pViewport->Height = height;
}

void Camera::SetScissorRect(const LONG width, const LONG height)
{
    pScissorRect->right = width;
    pScissorRect->bottom = height;
}

void Camera::UpdateCameraConstant()
{
    cameraConstant.WorldToProjectionMatrix = GetVPMatrix();
    XMStoreFloat4(&cameraConstant.CameraWorldPosition, worldPosition);
    cameraConstant.ProjectionToWorldMatrix = XMMatrixInverse(nullptr, GetVPMatrix());
    cameraConstant.TAAJitter.x = ((INT)ViewManager::sFrameCount % 4 - 1.5f) / width / 4.0f;
    cameraConstant.TAAJitter.y = ((INT)ViewManager::sFrameCount / 4 % 4 - 1.5f) / height / 4.0f;
    cameraConstant.FrameCount = ViewManager::sFrameCount;
}

const XMMATRIX Camera::GetVPMatrix()
{
    // Get the view matrix.
    XMMATRIX view = XMMatrixLookAtRH(worldPosition, worldPosition + forwardDirction, upDirction);

    // Get the proj matrix.
    XMMATRIX proj = XMMatrixPerspectiveFovRH(fov, aspectRatio, nearZ, farZ);

    return view * proj;
}
