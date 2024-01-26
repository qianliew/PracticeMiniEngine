#ifndef GBUFFER_VS_HLSL
#define GBUFFER_VS_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

PSInput VSMain(VSInput input)
{
    PSInput result;

    input.positionOS.w = 1;
    result.positionWS = mul(ObjectToWorldMatrix, input.positionOS);
    result.positionCS = mul(WorldToProjectionMatrix, result.positionWS);
    result.texCoord = float3(input.texCoord.xy, ObjectID);

    result.normalWS = normalize(GetWorldSpaceNormal(input.normalOS));
    result.tangentWS = float4(normalize(GetWorldSpaceTangent(input.tangentOS.xyz)), input.tangentOS.w);
    result.viewDirWS = normalize(GetWorldSpaceViewDir(result.positionWS));

    result.color = input.color;

    return result;
}

#endif
