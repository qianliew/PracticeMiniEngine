#pragma once

#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include "SharedConstants.h"

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

struct Ray
{
    XMFLOAT3 origin;
    XMFLOAT3 direction;
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

struct AABBAttributes
{
    FLOAT a;
};

#endif // !SHARED_TYPES_H
