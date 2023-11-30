#ifndef INPUTS_HLSLI
#define INPUTS_HLSLI

struct VSFullScreenInput
{
    float4 positionOS   : POSITION;
    float2 texCoord     : TEXCOORD;
};

struct PSFullScreenInput
{
    float4 positionCS   : SV_POSITION;
    float2 texCoord     : TEXCOORD;
};

#endif
