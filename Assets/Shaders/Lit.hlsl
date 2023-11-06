
cbuffer GlobalConstants : register(b0)
{
    float4x4 WorldToProjectionMatrix;
};

cbuffer PerObjectConstants : register(b1)
{
    float4x4 ObjectToWorldMatrix;
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
    float3 normalOS : NORMAL;
    float4 tangent  : TANGENT;
    float2 texCoord : TEXCOORD;
    float4 color    : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
    float3 normalWS : TEXCOORD1;
    float4 color    : COLOR;
};

float3 GetNormalizedWorldNormal(float3 normalOS)
{
    return normalize(mul((float3x3)ObjectToWorldMatrix, normalOS));
}

PSInput VSMain(VSInput input)
{
    PSInput result;

    input.position.w = 1;
    result.position = mul(mul(ObjectToWorldMatrix, WorldToProjectionMatrix), input.position);
    result.texCoord = input.texCoord;
    result.normalWS = GetNormalizedWorldNormal(input.normalOS);
    result.color = input.color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float NdotL = saturate(dot(input.normalWS, float3(1, 1, 0)));
    return t1.Sample(s1, input.texCoord) * NdotL;
}
