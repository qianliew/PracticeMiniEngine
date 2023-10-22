#include "stdafx.h"
#include "D3D12Camera.h"

D3D12Camera::D3D12Camera(FLOAT width, FLOAT height) :
    Transform(),
    width(width),
    height(height),
    fov(CAMERA_DEFAULT_FOV),
    aspectRatio(width / height),
    nearZ(CAMERA_DEFAULT_NEAR_Z),
    farZ(CAMERA_DEFAULT_FAR_Z)
{
    pViewport = new CD3DX12_VIEWPORT(0.0f, 0.0f, width, height);
    pScissorRect = new CD3DX12_RECT(0.0f, 0.0f, static_cast<LONG>(width), static_cast<LONG>(height));
    cameraConstantBuffer = std::make_shared<D3D12ConstantBuffer>((UINT)sizeof(CameraConstant));
}

D3D12Camera::~D3D12Camera()
{
    delete pViewport;
    delete pScissorRect;
}

void D3D12Camera::ResetTransform()
{
    Transform::ResetTransform();

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

const XMMATRIX D3D12Camera::GetVPMatrix()
{
    // Get the view matrix.
    XMMATRIX view = XMMatrixLookAtRH(worldPosition, worldPosition + forwardDirction, upDirction);

    // Get the proj matrix.
    XMMATRIX proj = XMMatrixPerspectiveFovRH(fov, aspectRatio, nearZ, farZ);

    return view * proj;
}
