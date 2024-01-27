#ifndef TEMPORALAA_VS_HLSL
#define TEMPORALAA_VS_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

PSFullScreenInput VSTemporalAA(VSFullScreenInput input)
{
    PSFullScreenInput result;

    result.positionCS = mul(IdentityProjectionMatrix, input.positionOS);
    result.texCoord = input.texCoord;

    return result;
}

#endif
