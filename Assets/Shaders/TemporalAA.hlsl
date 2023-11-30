#ifndef TEMPORALAA_HLSL
#define TEMPORALAA_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

Texture2D SourceTexture : register(t0);
SamplerState SourceTextureSampler : register(s0);

PSFullScreenInput VSTemporalAA(VSFullScreenInput input)
{
    PSFullScreenInput result;

    result.positionCS = mul(IdentityProjectionMatrix, input.positionOS);
    result.texCoord = input.texCoord;

    return result;
}

float4 PSTemporalAA(PSFullScreenInput input) : SV_TARGET
{
    return SourceTexture.Sample(SourceTextureSampler, input.texCoord) + 0.1f;
}

#endif
