#ifndef BLIT_PS_HLSL
#define BLIT_PS_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

Texture2D SourceTexture : register(t0);

float4 PSBlit(PSFullScreenInput input) : SV_TARGET
{
    return SourceTexture.Sample(StaticLinearClampSampler, input.texCoord);
}

#endif
