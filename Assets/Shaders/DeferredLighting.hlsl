#ifndef DEFERRED_LIGHTING_HLSL
#define DEFERRED_LIGHTING_HLSL

RWTexture2D<float4> Result : register(u0);

[numthreads(1, 1, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
	for (uint i = 0; i < 100; i++)
	{
		for (uint j = 0; j < 100; j++)
		{
			Result[uint2(threadID.x * i, threadID.y * j)] =
				float4(threadID.x * i / 500.0f, threadID.y * j / 500.0f, 1.0f, 1.0f);
		}
	}
}

#endif
