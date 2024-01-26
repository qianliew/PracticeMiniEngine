#ifndef TEMPORALAA_PS_HLSL
#define TEMPORALAA_PS_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

Texture2D SourceTexture : register(t0);
Texture2D TAAHistoryTexture : register(t1);
Texture2D DepthTexture : register(t2);

float4 PSTemporalAA(PSFullScreenInput input) : SV_TARGET
{
    float depth = DepthTexture.Sample(StaticLinearClampSampler, input.texCoord).r;
    float4 positionCS = float4(input.texCoord * 2.0f - 1.0f, depth, 1.0f);
    positionCS.y = -positionCS.y;
    float4 positionWS = mul(ProjectionToWorldMatrix, positionCS);
    positionWS /= positionWS.w;
    float4 positionCSHistory = mul(PreviousWorldToProjectionMatrix, positionWS);
    positionCSHistory.y = -positionCSHistory.y;
    float2 uvHistory = (positionCSHistory.xy / positionCSHistory.w + 1.0f) / 2.0f;

    const float alpha = 0.8f;
    float4 history = TAAHistoryTexture.Sample(StaticLinearClampSampler, uvHistory);

    // Upsample the color of the current frame.
    float4 color = SourceTexture.Sample(StaticLinearClampSampler, input.texCoord + TAAJitter.xy + float2(TAAJitter.z, -TAAJitter.w));
    color += SourceTexture.Sample(StaticLinearClampSampler, input.texCoord + TAAJitter.xy + float2(TAAJitter.z, 0.0f));
    color += SourceTexture.Sample(StaticLinearClampSampler, input.texCoord + TAAJitter.xy + float2(TAAJitter.z, TAAJitter.w));
    color += SourceTexture.Sample(StaticLinearClampSampler, input.texCoord + TAAJitter.xy + float2(0.0f, -TAAJitter.w));
    color += SourceTexture.Sample(StaticLinearClampSampler, input.texCoord + TAAJitter.xy + float2(0.0f, 0.0f));
    color += SourceTexture.Sample(StaticLinearClampSampler, input.texCoord + TAAJitter.xy + float2(0.0f, TAAJitter.w));
    color += SourceTexture.Sample(StaticLinearClampSampler, input.texCoord + TAAJitter.xy + float2(-TAAJitter.z, -TAAJitter.w));
    color += SourceTexture.Sample(StaticLinearClampSampler, input.texCoord + TAAJitter.xy + float2(-TAAJitter.z, 0.0f));
    color += SourceTexture.Sample(StaticLinearClampSampler, input.texCoord + TAAJitter.xy + float2(-TAAJitter.z, TAAJitter.w));
    color /= 9.0f;

    return lerp(color, history, alpha);
}

#endif
