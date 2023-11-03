
cbuffer GlobalConstants : register(b0)
{
    float4x4 ObjectToWorldMatrix;
};

cbuffer PerObjectConstants : register(b1)
{
    float4x4 WorldToProjectionMatrix;
};

Texture2D t1 : register(t1);
Texture2D t2 : register(t2);
Texture2D t3 : register(t3);
SamplerState s1 : register(s1);
SamplerState s2 : register(s2);
SamplerState s3 : register(s3);

struct VSInput
{
    float4 position : POSITION;
    float4 normal   : NORMAL;
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
    return t3.Sample(s3, input.texCoord);
    return t1.Sample(s1, input.texCoord) * 0.5f + t2.Sample(s2, input.texCoord) * 0.5f;
}
