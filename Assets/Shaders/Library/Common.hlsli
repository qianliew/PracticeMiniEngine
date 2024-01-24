#ifndef COMMON_HLSLI
#define COMMON_HLSLI

cbuffer GlobalConstants : register(b0)
{
    float4x4 WorldToProjectionMatrix;
    float4x4 ProjectionToWorldMatrix;
    float4x4 PreviousWorldToProjectionMatrix;
    float4x4 IdentityProjectionMatrix;
    float4 CameraPositionWS;
    float4 TAAJitter;
    uint FrameCount;
};

SamplerState StaticLinearClampSampler : register(s0);

cbuffer PerObjectConstants : register(b1)
{
    float4x4 ObjectToWorldMatrix;
    uint ObjectID;
};

inline float3 GetWorldSpaceNormal(float3 normalOS)
{
    return mul((float3x3)ObjectToWorldMatrix, normalOS);
}

inline float3 GetWorldSpaceTangent(float3 tangentOS)
{
    return mul((float3x3)ObjectToWorldMatrix, tangentOS);
}

inline float3 GetWorldSpaceViewDir(float3 positionWS)
{
    return CameraPositionWS.xyz - positionWS;
}

#endif
