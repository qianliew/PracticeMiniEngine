#pragma once

#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

struct CameraConstant
{
    XMMATRIX WorldToProjectionMatrix;
    XMMATRIX ProjectionToWorldMatrix;
    XMMATRIX PreviousWorldToProjectionMatrix;
    XMMATRIX IdentityProjectionMatrix;
    XMFLOAT4 CameraWorldPosition;
    XMFLOAT4 TAAJitter;
    UINT FrameCount;
};

struct Vertex
{
    XMFLOAT3 positionOS;
    XMFLOAT3 normalOS;
    XMFLOAT4 tangentOS;
    XMFLOAT2 texCoord;
    XMFLOAT4 color;
};

struct RayPayload
{
    XMFLOAT4 color;
    XMFLOAT3 direction;
    FLOAT attenuation;
    UINT depth;
    UINT randomSeed;
};

struct AORayPayload
{
    FLOAT aoVal;
};

struct GIRayPayload
{
    XMFLOAT4 color;
    XMFLOAT3 direction;
    UINT depth;
    UINT randomSeed;
};

struct ShadowRayPayload
{
    FLOAT attenuation;
};

struct FrustumCullingRayPayload
{
    FLOAT vis;
};

#endif // !SHARED_TYPES_H
