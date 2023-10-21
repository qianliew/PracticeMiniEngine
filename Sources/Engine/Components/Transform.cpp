#include "stdafx.h"
#include "Transform.h"

Transform::Transform() :
	worldPosition(DefaultWorldPosition),
	forwardDirction(DefaultForwardDirction),
	upDirction(DefaultUpDirction)
{
    transformConstantBuffer = std::make_shared<D3D12UploadBuffer>();
}

void Transform::ResetTransform()
{
	worldPosition = DefaultWorldPosition;
	forwardDirction = DefaultForwardDirction;
	upDirction = DefaultUpDirction;
}

void Transform::SetObjectToWorldMatrix()
{
    XMMATRIX m = XMMatrixTranslation(0.0f, 0.3f, 0.0f);
    XMStoreFloat4x4(&transformConstant.ObjectToWorldMatrix, m);
}

void Transform::MoveAlongZ(const FLOAT direction)
{
    worldPosition += forwardDirction * DEFAULT_MOVING_SPEED * direction;
}

void Transform::MoveAlongX(const FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(upDirction, forwardDirction);

    worldPosition += vec * DEFAULT_MOVING_SPEED * direction;
}

void Transform::RotateAlongY(const FLOAT direction)
{
    XMMATRIX trans = XMMatrixRotationNormal(g_XMIdentityR1, DEFAULT_ROTATING_SPEED * direction);
    forwardDirction = XMPlaneNormalize(XMVector3Transform(forwardDirction, trans));
    upDirction = XMPlaneNormalize(XMVector3Transform(upDirction, trans));
}

void Transform::RotateAlongX(const FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(upDirction, forwardDirction);

    XMMATRIX trans = XMMatrixRotationNormal(vec, DEFAULT_ROTATING_SPEED * direction);
    forwardDirction = XMPlaneNormalize(XMVector3Transform(forwardDirction, trans));
    upDirction = XMPlaneNormalize(XMVector3Transform(upDirction, trans));
}

void Transform::CreateView()
{
    cbvDesc.BufferLocation = transformConstantBuffer->ResourceLocation->Resource->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = CalculateConstantBufferByteSize(transformConstantBuffer->GetDataSize());
}
