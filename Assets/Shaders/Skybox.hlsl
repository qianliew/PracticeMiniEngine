
cbuffer GlobalConstants : register(b0)
{
    float4x4 WorldToProjectionMatrix;
    float3 CameraPositionWS;
};

cbuffer PerObjectConstants : register(b1)
{
    float4x4 ObjectToWorldMatrix;
};

TextureCube t1 : register(t1);
SamplerState s1 : register(s1);

struct VSInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    input.position.w = 1;
    result.position = mul(WorldToProjectionMatrix, mul(ObjectToWorldMatrix, input.position));
    result.texCoord = input.position.xyz;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return t1.Sample(s1, input.texCoord);
}
