#ifndef COMMON_HLSLI
#define COMMON_HLSLI

cbuffer GlobalConstants : register(b0)
{
    float4x4 WorldToProjectionMatrix;
    float4x4 ProjectionToWorldMatrix;
    float4x4 IdentityProjectionMatrix;
    float3 CameraPositionWS;
    float2 TAAJitter;
    uint FrameCount;
};

#endif
