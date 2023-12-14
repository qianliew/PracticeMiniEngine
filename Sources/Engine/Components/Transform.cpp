#include "stdafx.h"
#include "Transform.h"

Transform::Transform(UINT index) :
	worldPosition(DefaultWorldPosition),
	forwardDirction(DefaultForwardDirction),
	upDirction(DefaultUpDirction),
    id(index)
{

}

Transform::~Transform()
{

}

void Transform::ResetTransform()
{
	worldPosition = DefaultWorldPosition;
	forwardDirction = DefaultForwardDirction;
	upDirction = DefaultUpDirction;
}

void Transform::CopyWorldPosition(const Transform& other)
{
    this->worldPosition = other.worldPosition;
}

void Transform::SetObjectToWorldMatrix()
{
    XMMATRIX m = XMMatrixTranslationFromVector(worldPosition);
    XMStoreFloat4x4(&transformConstant.ObjectToWorldMatrix, m);
}

void Transform::MoveAlongX(const FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(upDirction, forwardDirction);

    worldPosition += vec * DEFAULT_MOVING_SPEED * direction;
}

void Transform::MoveAlongY(const FLOAT direction)
{
    worldPosition += upDirction * DEFAULT_MOVING_SPEED * direction;
}

void Transform::MoveAlongZ(const FLOAT direction)
{
    worldPosition += forwardDirction * DEFAULT_MOVING_SPEED * direction;
}

void Transform::RotateAlongY(const FLOAT direction)
{
    XMMATRIX trans = XMMatrixRotationNormal(g_XMIdentityR1, DEFAULT_ROTATING_SPEED * direction);
    forwardDirction = XMVector3Normalize(XMVector3Transform(forwardDirction, trans));
    upDirction = XMVector3Normalize(XMVector3Transform(upDirction, trans));
}

void Transform::RotateAlongX(const FLOAT direction)
{
    XMVECTOR vec = XMVector3Cross(upDirction, forwardDirction);

    XMMATRIX trans = XMMatrixRotationNormal(vec, DEFAULT_ROTATING_SPEED * direction);
    forwardDirction = XMVector3Normalize(XMVector3Transform(forwardDirction, trans));
    upDirction = XMVector3Normalize(XMVector3Transform(upDirction, trans));
}
