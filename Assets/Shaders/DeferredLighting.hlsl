#ifndef DEFERRED_LIGHTING_HLSL
#define DEFERRED_LIGHTING_HLSL

#include "Library/BRDF.hlsli"
#include "Library/Common.hlsli"

RWTexture2D<float4> Result : register(u0);

Texture2D GBuffer0 : register(t10);
Texture2D GBuffer1 : register(t11);
Texture2D GBuffer2 : register(t12);
Texture2D GBuffer3 : register(t13);

[numthreads(10, 10, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
	float2 uv = threadID.xy * TAAJitter.zw;
	float4 baseColor = GBuffer0.SampleLevel(StaticLinearClampSampler, uv, 0.0f);
	float4 attributes = GBuffer1.SampleLevel(StaticLinearClampSampler, uv, 0.0f);
	float3 normalWS = GBuffer2.SampleLevel(StaticLinearClampSampler, uv, 0.0f).rgb;
	float3 positionWS = GBuffer3.SampleLevel(StaticLinearClampSampler, uv, 0.0f).rgb;

	float a = attributes.y; // roughness
	float a2 = a * a; // roughness square
	float3 viewDirWS = normalize(GetWorldSpaceViewDir(positionWS));
	float3 lightDirWS = normalize(float3(1.0f, 1.0f, 0.0f));
	float3 halfDir = normalize(lightDirWS + viewDirWS);
	float NdotH = saturate(dot(normalWS, halfDir));
	float NdotL = saturate(dot(normalWS, lightDirWS));
	float NdotV = max(dot(normalWS, viewDirWS), 0.0001f);
	float LdotH = saturate(dot(lightDirWS, halfDir));

	float d = NdotH * NdotH * (a2 - 1.0f) + 1.00001f;
	float specularTerm = a2 / ((d * d) * max(0.1f, LdotH * LdotH) * (a * 4.0f + 2.0f));
	float diffuseTerm = 1.0f;

	Result[threadID.xy] = float4((diffuseTerm + specularTerm) * baseColor.xyz * NdotL, 1.0f);
}

#endif
