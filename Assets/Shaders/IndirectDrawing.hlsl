#ifndef INDIRECT_DRAWING_HLSL
#define INDIRECT_DRAWING_HLSL

#define HLSL

#include "Library/Common.hlsli"

struct IndirectCommand
{
	uint2 cbv;
	uint4 vbv;
	uint4 ibv;
	uint3x2 drawArguments;
};

RWStructuredBuffer<uint> VisData : register(u10);

StructuredBuffer<IndirectCommand> commandBuffer : register(t20);
AppendStructuredBuffer<IndirectCommand> argumentBuffer : register(u1);

[numthreads(1, 1, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
	const uint max = 16;
	for (uint i = 0; i < max; i++)
	{
		argumentBuffer.Append(commandBuffer[i]);
	}
}

#endif
