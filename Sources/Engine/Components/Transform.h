#pragma once

#include "D3D12ConstantBuffer.h"

#define DEFAULT_MOVING_SPEED 2.0f
#define DEFAULT_ROTATING_SPEED XM_PI / 36.0f

using namespace DirectX;
using std::shared_ptr;

struct TransformConstant
{
    XMFLOAT4X4 ObjectToWorldMatrix;
};

class Transform
{
protected:
    const XMVECTOR DefaultWorldPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    const XMVECTOR DefaultForwardDirction = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    const XMVECTOR DefaultUpDirction = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMVECTOR worldPosition;
    XMVECTOR forwardDirction;
    XMVECTOR upDirction;

    UINT id;
    TransformConstant transformConstant;

public:
    Transform(UINT);
    virtual ~Transform();
    
    void CopyWorldPosition(const Transform &other);
    void SetObjectToWorldMatrix();

    virtual void ResetTransform();
    virtual void MoveAlongX(const FLOAT direction);
    virtual void MoveAlongY(const FLOAT direction);
    virtual void MoveAlongZ(const FLOAT direction);
    virtual void RotateAlongY(const FLOAT direction);
    virtual void RotateAlongX(const FLOAT direction);

    inline const UINT GetObjectID() const { return id; }
    inline XMVECTOR GetWorldPosition() const { return worldPosition; }
    inline TransformConstant& GetTransformConstant() { return transformConstant; }
};
