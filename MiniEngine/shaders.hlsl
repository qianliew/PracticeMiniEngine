//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

cbuffer GlobalConstants : register(b0)
{
    float4x4 ObjectToWorldMatrix;
};

cbuffer PerObjectConstants : register(b1)
{
    float4x4 WorldToProjectionMatrix;
};

Texture2D t1 : register(t0);
SamplerState s1 : register(s0);

struct VSInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
    float4 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float4 color    : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    input.position.w = 1;
    result.position = mul(mul(ObjectToWorldMatrix, WorldToProjectionMatrix), input.position);
    result.texCoord = input.texCoord;
    result.color = input.color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return t1.Sample(s1, input.texCoord);
}
