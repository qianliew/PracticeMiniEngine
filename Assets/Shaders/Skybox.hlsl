#ifndef SKYBOX_HLSL
#define SKYBOX_HLSL

#include "Library/Common.hlsli"

cbuffer PerObjectConstants : register(b1)
{
    float4x4 ObjectToWorldMatrix;
};

TextureCube SkyboxCube : register(t1);
SamplerState SkyboxSampler : register(s1);

struct VSInput
{
    float4 positionOS   : POSITION;
    float2 texCoord     : TEXCOORD;
};

struct PSInput
{
    float4 positionCS   : SV_POSITION;
    float3 texCoord     : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    result.positionCS = mul(WorldToProjectionMatrix, mul(ObjectToWorldMatrix, input.positionOS));
    result.texCoord = input.positionOS.xyz;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return SkyboxCube.Sample(SkyboxSampler, input.texCoord);
}

#endif
