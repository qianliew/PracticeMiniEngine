
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

    input.positionOS /= 100;
    input.positionOS.z = 0;
    input.positionOS.w = 1;
    result.positionCS = input.positionOS;
    result.texCoord = input.texCoord;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return SourceTexture.Sample(SourceTextureSampler, input.texCoord);
}
