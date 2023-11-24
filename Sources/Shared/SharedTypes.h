#pragma once

#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

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
    UINT depth;
    UINT randomSeed;
};

struct AORayPayload
{
    FLOAT aoVal;
};

#endif // !SHARED_TYPES_H
