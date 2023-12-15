#ifndef BRDF_HLSLI
#define BRDF_HLSLI

#define INV_PI      0.31830988618379067154

inline float Square(float value)
{
    return value * value;
}

inline float SafeDiv(float numer, float denom)
{
    return (numer != denom) ? numer / denom : 1.0f;
}

float Lambert()
{
    return INV_PI;
}

float GGX(float NdotH, float roughness)
{
    float a = Square(roughness);
    float d = (NdotH * a - NdotH) * NdotH + 1.0f;

    return INV_PI * SafeDiv(a, d * d);
}

float SmithJoint(float NdotL, float NdotV, float roughness)
{
    float a = Square(roughness);
    float lambdaV = NdotL * (NdotV * (1.0f - a) + a);
    float lambdaL = NdotV * (NdotL * (1.0f - a) + a);

    return 0.5f * rcp(lambdaV + lambdaL);
}

float Schlick(float f0, float f90, float VdotH)
{
    float x = 1.0f - VdotH;
    float x2 = x * x;
    float x5 = x * x2 * x2;
    return (f90 - f0) * x5 + f0;
}

#endif
