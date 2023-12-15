#ifndef DEFERRED_LIGHTING_HLSL
#define DEFERRED_LIGHTING_HLSL

#include "Library/Common.hlsli"

RWTexture2D<float4> Result : register(u0);

Texture2D GBuffer0 : register(t10);
Texture2D GBuffer1 : register(t11);
Texture2D GBuffer2 : register(t12);

[numthreads(10, 10, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
	float2 uv = threadID.xy / float2(1920.0f, 1080.0f);
	Result[threadID.xy] = GBuffer0.SampleLevel(SourceTextureSampler, uv, 0.0f);
}

#endif
