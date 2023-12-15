#ifndef BLIT_HLSL
#define BLIT_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

Texture2D SourceTexture : register(t0);

PSFullScreenInput VSBlit(VSFullScreenInput input)
{
    PSFullScreenInput result;

    result.positionCS = mul(IdentityProjectionMatrix, input.positionOS);
    result.texCoord = input.texCoord;

    return result;
}

float4 PSBlit(PSFullScreenInput input) : SV_TARGET
{
    return SourceTexture.Sample(SourceTextureSampler, input.texCoord);
}

#endif
