
cbuffer GlobalConstants : register(b0)
{
    float4x4 WorldToProjectionMatrix;
    float3 CameraPositionWS;
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
    float4 positionOS   : POSITION;
    float3 normalOS     : NORMAL;
    float4 tangentOS    : TANGENT;
    float2 texCoord     : TEXCOORD;
    float4 color        : COLOR;
};

struct PSInput
{
    float4 positionWS   : SV_POSITION;
    float2 texCoord     : TEXCOORD;
    float3 normalWS     : TEXCOORD1;
    float4 tangentWS    : TEXCOORD2;
    float3 viewDirWS    : TEXCOORD3;
    float4 color        : COLOR;
};

inline float3 GetWorldSpaceNormal(float3 normalOS)
{
    return mul((float3x3)ObjectToWorldMatrix, normalOS);
}

inline float3 GetWorldSpaceTangent(float3 tangentOS)
{
    return mul((float3x3)ObjectToWorldMatrix, tangentOS);
}

inline float3 GetWorldSpaceViewDir(float3 positionWS)
{
    return CameraPositionWS - positionWS;
}

PSInput VSMain(VSInput input)
{
    PSInput result;

    input.positionOS.w = 1;
    result.positionWS = mul(mul(ObjectToWorldMatrix, WorldToProjectionMatrix), input.positionOS);
    result.texCoord = input.texCoord;

    result.normalWS = normalize(GetWorldSpaceNormal(input.normalOS));
    result.tangentWS = float4(normalize(GetWorldSpaceTangent(input.tangentOS.xyz)), input.tangentOS.w);
    result.viewDirWS = normalize(GetWorldSpaceViewDir(result.positionWS));

    result.color = input.color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 normalTS = t3.Sample(s3, input.texCoord).xyz * 2.0f - 1.0f;

    float sgn = input.tangentWS.w > 0.0f ? -1.0f : 1.0f;
    float3 bitangentWS = sgn * cross(input.normalWS.xyz, input.tangentWS.xyz);
    float3 normalWS = mul(normalTS, float3x3(input.tangentWS.xyz, bitangentWS.xyz, input.normalWS.xyz));

    float roughness = t2.Sample(s2, input.texCoord).g;
    float roughness2 = roughness * roughness;

    float3 diffuse = t1.Sample(s1, input.texCoord).rgb;
    float3 specular = diffuse;

    float3 lightDirWS = float3(1.0f, 1.0f, 0.0f);
    float NdotV = dot(normalWS, input.viewDirWS);
    float NdotL = dot(normalWS, lightDirWS);

    float clampedNdotV = max(NdotV, 0.0001f);
    float clampedNdotL = saturate(NdotL);

    float diffuseTerm = 1.0f;

    float3 halfDir = normalize(lightDirWS + input.viewDirWS);
    float NdotH = saturate(dot(normalWS, halfDir));
    float LdotH = saturate(dot(lightDirWS, halfDir));

    float d = NdotH * NdotH * (roughness2 - 1.0f) + 1.00001f;

    float LdotH2 = LdotH * LdotH;
    float specularTerm = roughness2 / ((d * d) * max(0.1h, LdotH2) * (roughness * 4.0f + 2.0f));

    return float4((diffuse * diffuseTerm + specular * specularTerm) * clampedNdotL, 1.0f);
}
