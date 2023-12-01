#ifndef TEMPORALAA_HLSL
#define TEMPORALAA_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

Texture2D SourceTexture : register(t0);
Texture2D TAAHistoryTexture : register(t1);
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
    const float alpha = 0.7f;
    float4 history = TAAHistoryTexture.Sample(SourceTextureSampler, input.texCoord);
    float4 color = SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter);

    return lerp(color, history, alpha);
}

#endif
