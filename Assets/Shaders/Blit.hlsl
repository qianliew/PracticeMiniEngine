
cbuffer GlobalConstants : register(b0)
{
    float4x4 WorldToProjectionMatrix;
    float4x4 ProjectionToWorldMatrix;
    float4x4 IdentityProjectionMatrix;
    float3 CameraPositionWS;
    uint FrameCount;
};

Texture2D SourceTexture : register(t0);
SamplerState SourceTextureSampler : register(s0);

struct VSInput
{
    float4 positionOS   : POSITION;
    float2 texCoord     : TEXCOORD;
};

struct PSInput
{
    float4 positionCS   : SV_POSITION;
    float2 texCoord     : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput result;

    result.positionCS = mul(IdentityProjectionMatrix, input.positionOS);
    result.texCoord = input.texCoord;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return SourceTexture.Sample(SourceTextureSampler, input.texCoord);
}
