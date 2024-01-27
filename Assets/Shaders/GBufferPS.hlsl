#ifndef GBUFFER_PS_HLSL
#define GBUFFER_PS_HLSL

#include "Library/Common.hlsli"
#include "Library/Inputs.hlsli"

Texture2D BaseTexture   : register(t5);
Texture2D MRATexture    : register(t6);
Texture2D NormalTexture : register(t7);
Texture2DArray BaseTextures     : register(t8);
Texture2DArray MRATextures      : register(t9);
Texture2DArray NormalTextures   : register(t10);

SamplerState BaseTextureSampler     : register(s5);
SamplerState MRATextureSampler      : register(s6);
SamplerState NormalTextureSampler   : register(s7);

void PSMain(PSInput input,
    out float4 GBuffer0 : SV_TARGET0,
    out float4 GBuffer1 : SV_TARGET1,
    out float4 GBuffer2 : SV_TARGET2,
    out float4 GBuffer3 : SV_TARGET3)
{
    GBuffer0 = BaseTextures.Sample(BaseTextureSampler, input.texCoord);
    GBuffer1 = MRATextures.Sample(MRATextureSampler, input.texCoord);

    float3 normalTS = NormalTextures.Sample(NormalTextureSampler, input.texCoord).xyz * 2.0f - 1.0f;
    float sgn = input.tangentWS.w > 0.0f ? 1.0f : -1.0f;
    float3 bitangentWS = sgn * cross(input.normalWS.xyz, input.tangentWS.xyz);
    float3 normalWS = mul(normalTS, float3x3(input.tangentWS.xyz, bitangentWS.xyz, input.normalWS.xyz));
    GBuffer2 = float4(normalize(normalWS), 0.0f);
    GBuffer3 = input.positionWS;
}

#endif
