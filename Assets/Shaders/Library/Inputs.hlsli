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
    float3 texCoord     : TEXCOORD0;
    float3 normalWS     : TEXCOORD1;
    float4 tangentWS    : TEXCOORD2;
    float3 viewDirWS    : TEXCOORD3;
    float4 positionWS   : TEXCOORD4;
    float4 color        : COLOR;
};

#endif
