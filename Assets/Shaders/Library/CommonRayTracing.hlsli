#ifndef COMMON_RAY_TRACING_HLSLI
#define COMMON_RAY_TRACING_HLSLI

#include "Common.hlsli"

inline void GetRay(out float3 origin, out float3 direction)
{
    // Orthographic projection since we're raytracing in screen space.
    origin = CameraPositionWS.xyz;

    float2 xy = DispatchRaysIndex().xy + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a world positon.
    float4 world = mul(ProjectionToWorldMatrix, float4(screenPos, 0, 1));
    world.xyz /= world.w;

    direction = normalize(world.xyz - origin);
}

inline float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

inline float3 GetBarycentrics(float2 barycentrics)
{
    return float3(1 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);
}

float3 GetPerpencdicularVector(float3 u)
{
    float3 a = abs(u);
    uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
    uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
    uint zm = 1 ^ (xm | ym);
    return cross(u, float3(xm, ym, zm));
}

float3 GetCosHemisphereSample(float2 randVal, float3 hitNorm)
{
    // Cosine weighted hemisphere sample from RNG
    float3 bitangent = GetPerpencdicularVector(hitNorm);
    float3 tangent = cross(bitangent, hitNorm);
    float r = sqrt(randVal.x);
    float phi = 2.0f * 3.14159265f * randVal.y;

    // Get our cosine-weighted hemisphere lobe sample direction
    return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) + hitNorm.xyz * sqrt(1 - randVal.x);
}

#endif
