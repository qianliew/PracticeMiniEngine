#ifndef GBUFFER_HLSL
#define GBUFFER_HLSL

#include "Library/Common.hlsli"

Texture2D BaseTexture   : register(t5);
Texture2D MRATexture    : register(t6);
Texture2D NormalTexture : register(t7);
Texture2DArray BaseTextures     : register(t8);
Texture2DArray MRATextures      : register(t9);
Texture2DArray NormalTextures   : register(t10);

SamplerState BaseTextureSampler     : register(s5);
SamplerState MRATextureSampler      : register(s6);
SamplerState NormalTextureSampler   : register(s7);

struct VSInput
{
    float4 positionOS   : POSITION;
    float3 normalOS     : NORMAL;
    float4 tangentOS    : TANGENT;
    float2 texCoord     : TEXCOORD;
    float4 color        : COLOR;
};

struct PSInput
{
    float4 positionCS   : SV_POSITION;
    float2 texCoord     : TEXCOORD0;
    float3 normalWS     : TEXCOORD1;
    float4 tangentWS    : TEXCOORD2;
    float3 viewDirWS    : TEXCOORD3;
    float4 positionWS   : TEXCOORD4;
    float4 color        : COLOR;
    uint   instanceID   : SV_InstanceID;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    input.positionOS.w = 1;
    result.positionWS = mul(ObjectToWorldMatrix, input.positionOS);
    result.positionCS = mul(WorldToProjectionMatrix, result.positionWS);
    result.texCoord = input.texCoord;

    result.normalWS = normalize(GetWorldSpaceNormal(input.normalOS));
    result.tangentWS = float4(normalize(GetWorldSpaceTangent(input.tangentOS.xyz)), input.tangentOS.w);
    result.viewDirWS = normalize(GetWorldSpaceViewDir(result.positionWS));

    result.color = input.color;

    return result;
}

void PSMain(PSInput input,
    out float4 GBuffer0 : SV_TARGET0,
    out float4 GBuffer1 : SV_TARGET1,
    out float4 GBuffer2 : SV_TARGET2,
    out float4 GBuffer3 : SV_TARGET3)
{
    GBuffer0 = BaseTextures.Sample(StaticLinearClampSampler, input.texCoord, instanceID);
    GBuffer1 = MRATexture.Sample(StaticLinearClampSampler, input.texCoord, instanceID);

    float3 normalTS = NormalTexture.Sample(StaticLinearClampSampler, input.texCoord, instanceID).xyz * 2.0f - 1.0f;
    float sgn = input.tangentWS.w > 0.0f ? 1.0f : -1.0f;
    float3 bitangentWS = sgn * cross(input.normalWS.xyz, input.tangentWS.xyz);
    float3 normalWS = mul(normalTS, float3x3(input.tangentWS.xyz, bitangentWS.xyz, input.normalWS.xyz));
    GBuffer2 = float4(normalize(normalWS), 0.0f);
    GBuffer3 = input.positionWS;
}

#endif
