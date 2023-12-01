#pragma once

#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

struct CameraConstant
{
    XMMATRIX WorldToProjectionMatrix;
    XMMATRIX ProjectionToWorldMatrix;
    XMMATRIX IdentityProjectionMatrix;
    XMFLOAT3 CameraWorldPosition;
    XMFLOAT2 TAAJitter;
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

#endif // !SHARED_TYPES_H
