#pragma once

#define DEFAULT_MOVING_SPEED 2.0f
#define DEFAULT_ROTATING_SPEED XM_PI / 36.0f

using namespace DirectX;

struct Matrices
{
    XMFLOAT4X4 ObjectToWorldMatrix;
    XMFLOAT4X4 WorldToProjectionMatrix;
};

class Transform
{
protected:
    const XMVECTOR DefaultWorldPosition = XMVectorSet(0, 0, -50, 1);
    const XMVECTOR DefaultForwardDirction = XMVectorSet(0, 0, 1, 0);
    const XMVECTOR DefaultUpDirction = XMVectorSet(0, 1, 0, 0);

    XMVECTOR worldPosition;
    XMVECTOR forwardDirction;
    XMVECTOR upDirction;

    Matrices matrices;

public:
    Transform();

    void SetObjectToWorldMatrix();

    virtual void ResetTransform();
    virtual void MoveAlongZ(const FLOAT direction);
    virtual void MoveAlongX(const FLOAT direction);
    virtual void RotateAlongY(const FLOAT direction);
    virtual void RotateAlongX(const FLOAT direction);

    Matrices& GetMatrices() { return matrices; }
};
