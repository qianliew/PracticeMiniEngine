#pragma once

#ifndef SHARED_PRIMITIVES_H
#define SHARED_PRIMITIVES_H

#ifdef HLSL
typedef float FLOAT;
typedef float2 XMFLOAT2;
typedef float3 XMFLOAT3;
typedef float4 XMFLOAT4;
typedef float4 XMVECTOR;
typedef float4x4 XMMATRIX;
typedef uint UINT;
typedef uint2 D3D12_GPU_VIRTUAL_ADDRESS;
typedef uint4 D3D12_DRAW_ARGUMENTS;

#else
using namespace DirectX;

#endif

#endif // !SHARED_PRIMITIVES_H
