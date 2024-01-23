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

#ifndef HLSL
    bool operator==(const Vertex& vertex) const
    {
        return (
            positionOS.x == vertex.positionOS.x
            && positionOS.y == vertex.positionOS.y
            && positionOS.z == vertex.positionOS.z
            && normalOS.x == vertex.normalOS.x
            && normalOS.y == vertex.normalOS.y
            && normalOS.z == vertex.normalOS.z
            && tangentOS.x == vertex.tangentOS.x
            && tangentOS.y == vertex.tangentOS.y
            && tangentOS.z == vertex.tangentOS.z
            && tangentOS.w == vertex.tangentOS.w
            && texCoord.x == vertex.texCoord.x
            && texCoord.y == vertex.texCoord.y);
    }
#endif
};

struct IndirectCommand
{
    D3D12_GPU_VIRTUAL_ADDRESS cbv;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    D3D12_INDEX_BUFFER_VIEW ibv;
    D3D12_DRAW_INDEXED_ARGUMENTS drawArguments;
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
