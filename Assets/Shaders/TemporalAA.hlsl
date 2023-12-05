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
    const float alpha = 0.5f;
    float4 history = TAAHistoryTexture.Sample(SourceTextureSampler, input.texCoord);

    // Upsample the color of the current frame.
    float4 color = SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter.xy + float2(TAAJitter.z, -TAAJitter.w));
    color += SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter.xy + float2(TAAJitter.z, 0.0f));
    color += SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter.xy + float2(TAAJitter.z, TAAJitter.w));
    color += SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter.xy + float2(0.0f, -TAAJitter.w));
    color += SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter.xy + float2(0.0f, 0.0f));
    color += SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter.xy + float2(0.0f, TAAJitter.w));
    color += SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter.xy + float2(-TAAJitter.z, -TAAJitter.w));
    color += SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter.xy + float2(-TAAJitter.z, 0.0f));
    color += SourceTexture.Sample(SourceTextureSampler, input.texCoord + TAAJitter.xy + float2(-TAAJitter.z, TAAJitter.w));
    color /= 9.0f;

    return lerp(color, history, alpha);
}

#endif
