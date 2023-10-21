#pragma once

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
    const XMVECTOR DefaultWorldPosition = XMVectorSet(0, 0, -50, 1);
    const XMVECTOR DefaultForwardDirction = XMVectorSet(0, 0, 1, 0);
    const XMVECTOR DefaultUpDirction = XMVectorSet(0, 1, 0, 0);

    XMVECTOR worldPosition;
    XMVECTOR forwardDirction;
    XMVECTOR upDirction;

    TransformConstant transformConstant;

    shared_ptr<D3D12UploadBuffer> transformConstantBuffer;
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;

public:
    Transform();

    void SetObjectToWorldMatrix();

    virtual void ResetTransform();
    virtual void MoveAlongZ(const FLOAT direction);
    virtual void MoveAlongX(const FLOAT direction);
    virtual void RotateAlongY(const FLOAT direction);
    virtual void RotateAlongX(const FLOAT direction);

    TransformConstant& GetTransformConstant() { return transformConstant; }

    void CreateView();
    const shared_ptr<D3D12UploadBuffer> GetTransformConstantBuffer() const { return transformConstantBuffer; }
    const D3D12_CONSTANT_BUFFER_VIEW_DESC GetCBVDesc() const { return cbvDesc; }
};
