#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "../../Sources/Shared/SharedPrimitives.h"
#include "../../Sources/Shared/SharedTypes.h"
#include "../../Sources/Shared/SharedConstants.h"

RaytracingAccelerationStructure Scene : register(t0);
RWTexture2D<float4> RenderTarget : register(u0);

cbuffer GlobalConstants : register(b0)
{
    float4x4 WorldToProjectionMatrix;
    float4x4 ProjectionToWorldMatrix;
    float3 CameraPositionWS;
    uint FrameCount;
};

StructuredBuffer<uint16_t> Indices : register(t1);
StructuredBuffer<Vertex> Vertices : register(t2);
StructuredBuffer<uint> Offsets : register(t3);

uint initRand(uint val0, uint val1, uint backoff = 16)
{
    uint v0 = val0, v1 = val1, s0 = 0;

    [unroll]
    for (uint n = 0; n < backoff; n++)
    {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}

// Takes our seed, updates it, and returns a pseudorandom float in [0..1]
float nextRand(inout uint s)
{
    s = (1664525u * s + 1013904223u);
    return float(s & 0x00FFFFFF) / float(0x01000000);
}

float3 getPerpendicularVector(float3 u)
{
    float3 a = abs(u);
    uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
    uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
    uint zm = 1 ^ (xm | ym);
    return cross(u, float3(xm, ym, zm));
}

float3 getCosHemisphereSample(inout uint randSeed, float3 hitNorm)
{
    // Get 2 random numbers to select our sample with
    float2 randVal = float2(nextRand(randSeed), nextRand(randSeed));

    // Cosine weighted hemisphere sample from RNG
    float3 bitangent = getPerpendicularVector(hitNorm);
    float3 tangent = cross(bitangent, hitNorm);
    float r = sqrt(randVal.x);
    float phi = 2.0f * 3.14159265f * randVal.y;

    // Get our cosine-weighted hemisphere lobe sample direction
    return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) + hitNorm.xyz * sqrt(1 - randVal.x);
}

float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

float TraceAORay(float3 origin, float3 direction, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= RaytracingConstants::MaxRayRecursiveDepth)
    {
        return false;
    }

    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000.0;

    uint flags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
        | RAY_FLAG_FORCE_OPAQUE             // ~skip any hit shaders
        | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER; // ~skip closest hit shaders

    AORayPayload aoRayPayload = { 0.0f };
    TraceRay(Scene, flags, 0xFF, RayType::AO, 0, RayType::AO, rayDesc, aoRayPayload);

    return aoRayPayload.aoVal;
}

[shader("raygeneration")]
void RaygenShader()
{
    // Orthographic projection since we're raytracing in screen space.
    float3 origin = CameraPositionWS.xyz;

    float2 xy = DispatchRaysIndex().xy + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a world positon.
    float4 world = mul(ProjectionToWorldMatrix, float4(screenPos, 0, 1));
    world.xyz /= world.w;

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(world.xyz - origin);
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    uint currentRecursionDepth = 0;
    RayPayload payload = { float4(0, 0, 0, 0), currentRecursionDepth, xy.x + xy.y * DispatchRaysDimensions().x };

    // uint flags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    uint flags = RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

    TraceRay(Scene, flags, 0xFF, RayType::Radiance, 0, RayType::Radiance, ray, payload);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void ClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    if (payload.depth > RaytracingConstants::MaxRayRecursiveDepth)
    {
        return;
    }

    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    uint vertId = 3 * PrimitiveIndex() + Offsets[GeometryIndex()];
    float3 normalOS = Vertices[vertId + 0].normalOS * barycentrics.x +
        Vertices[vertId + 1].normalOS * barycentrics.y +
        Vertices[vertId + 2].normalOS * barycentrics.z;

    float3 hitPosition = HitWorldPosition();
    const uint rayCount = 30;
    for (uint i = 0; i < rayCount; i++)
    {
        uint seed = initRand(payload.randomSeed * rayCount + i, FrameCount, 16);
        float3 direction = normalize(getCosHemisphereSample(seed, normalOS));
        payload.color += TraceAORay(hitPosition, direction, payload.depth) / rayCount;
    }
}

[shader("closesthit")]
void AOClosestHitShader(inout AORayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.aoVal = 0.0f;
}

[shader("miss")]
void MissShader(inout RayPayload payload)
{
    payload.color = float4(0, 0, 0, 1);
}

[shader("miss")]
void AOMissShader(inout AORayPayload payload)
{
    payload.aoVal = 1.0f;
}

#endif // RAYTRACING_HLSL
