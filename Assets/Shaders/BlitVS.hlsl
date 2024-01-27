#ifndef BLIT_VS_HLSL
#define BLIT_VS_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

PSFullScreenInput VSBlit(VSFullScreenInput input)
{
    PSFullScreenInput result;

    result.positionCS = mul(IdentityProjectionMatrix, input.positionOS);
    result.texCoord = input.texCoord;

    return result;
}

#endif
