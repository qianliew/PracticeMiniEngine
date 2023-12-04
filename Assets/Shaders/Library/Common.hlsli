#ifndef COMMON_HLSLI
#define COMMON_HLSLI

cbuffer GlobalConstants : register(b0)
{
    float4x4 WorldToProjectionMatrix;
    float4x4 ProjectionToWorldMatrix;
    float4x4 IdentityProjectionMatrix;
    float4 CameraPositionWS;
    float4 TAAJitter;
    uint FrameCount;
};

#endif
