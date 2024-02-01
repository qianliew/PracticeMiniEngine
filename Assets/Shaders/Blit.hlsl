#ifndef BLIT_HLSL
#define BLIT_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

Texture3D SourceTexture : register(t0);

PSFullScreenInput VSBlit(VSFullScreenInput input)
{
    PSFullScreenInput result;

    result.positionCS = mul(IdentityProjectionMatrix, input.positionOS);
    result.texCoord = input.texCoord;

    return result;
}

float4 PSBlit(PSFullScreenInput input) : SV_TARGET
{
    return SourceTexture.SampleLevel(StaticLinearClampSampler, float3(input.texCoord.xy, 0.0f), 1.0f);
}

#endif
