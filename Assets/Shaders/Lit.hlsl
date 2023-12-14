#ifndef LIT_HLSL
#define LIT_HLSL

#include "Library/Common.hlsli"

Texture2D BaseTexture   : register(t5);
Texture2D MRATexture    : register(t6);
Texture2D NormalTexture : register(t7);

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

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 normalTS = NormalTexture.Sample(NormalTextureSampler, input.texCoord).xyz * 2.0f - 1.0f;

    float sgn = input.tangentWS.w > 0.0f ? -1.0f : 1.0f;
    float3 bitangentWS = sgn * cross(input.normalWS.xyz, input.tangentWS.xyz);
    float3 normalWS = mul(normalTS, float3x3(input.tangentWS.xyz, bitangentWS.xyz, input.normalWS.xyz));

    float roughness = MRATexture.Sample(MRATextureSampler, input.texCoord).g;
    float roughness2 = roughness * roughness;

    float3 diffuse = BaseTexture.Sample(BaseTextureSampler, input.texCoord).rgb;
    float3 specular = diffuse;

    float3 lightDirWS = float3(1.0f, 1.0f, 0.0f);
    float NdotV = dot(normalWS, input.viewDirWS);
    float NdotL = dot(normalWS, lightDirWS);

    float clampedNdotV = max(NdotV, 0.0001f);
    float clampedNdotL = saturate(NdotL);

    float diffuseTerm = 1.0f;

    float3 halfDir = normalize(lightDirWS + input.viewDirWS);
    float NdotH = saturate(dot(normalWS, halfDir));
    float LdotH = saturate(dot(lightDirWS, halfDir));

    float d = NdotH * NdotH * (roughness2 - 1.0f) + 1.00001f;

    float LdotH2 = LdotH * LdotH;
    float specularTerm = roughness2 / ((d * d) * max(0.1h, LdotH2) * (roughness * 4.0f + 2.0f));

    return float4((diffuse * diffuseTerm + specular * specularTerm) * clampedNdotL, 1.0f);
}

#endif
