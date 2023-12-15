#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL

#include "Library/Common.hlsli"
#include "../../Sources/Shared/SharedPrimitives.h"
#include "../../Sources/Shared/SharedTypes.h"
#include "../../Sources/Shared/SharedConstants.h"

RaytracingAccelerationStructure Scene : register(t0);
RWTexture2D<float4> Result : register(u0);

StructuredBuffer<uint16_t> Indices : register(t1);
StructuredBuffer<Vertex> Vertices : register(t2);
StructuredBuffer<uint> Offsets : register(t3);

TextureCube SkyboxCube  : register(t4);

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

inline float3 GetBarycentrics(float2 barycentrics)
{
    return float3(1 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);
}

float4 TraceRadianceRay(float3 origin, float3 direction, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= RaytracingConstants::MaxRayRecursiveDepth)
    {
        return 0.0f;
    }

    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000.0;

    uint flags = RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

    RayPayload payload = 
    {
        float4(0, 0, 0, 0),
        direction,
        currentRayRecursionDepth,
        DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x
    };
    TraceRay(Scene, flags, 0xFF, RayType::Radiance, 0, RayType::Radiance, rayDesc, payload);

    return payload.color;
}

float TraceAORay(float3 origin, float3 direction, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= RaytracingConstants::MaxRayRecursiveDepth)
    {
        return 0.0f;
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

float3 TraceGIRay(float3 origin, float3 direction, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= RaytracingConstants::MaxRayRecursiveDepth)
    {
        return 0.0f;
    }

    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000.0;

    uint flags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
        | RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

    GIRayPayload payload =
    {
        float4(0, 0, 0, 0),
        direction,
        currentRayRecursionDepth,
        DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x
    };
    TraceRay(Scene, flags, 0xFF, RayType::GI, 0, RayType::GI, rayDesc, payload);

    return payload.color.rgb;
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

    uint currentRayRecursionDepth = 0;
    Result[DispatchRaysIndex().xy] +=
        TraceRadianceRay(origin, normalize(world.xyz - origin), currentRayRecursionDepth);
}

[shader("closesthit")]
void ClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.depth += 1;
    float3 barycentrics = GetBarycentrics(attr.barycentrics);
    uint vertId = 3 * PrimitiveIndex() + Offsets[GeometryIndex()];

    float4 hitPosition = float4(HitWorldPosition(), 1.0f);
    float3 normalOS = Vertices[vertId + 0].normalOS * barycentrics.x +
        Vertices[vertId + 1].normalOS * barycentrics.y +
        Vertices[vertId + 2].normalOS * barycentrics.z;
    float2 uv = Vertices[vertId + 0].texCoord * barycentrics.x +
        Vertices[vertId + 1].texCoord * barycentrics.y +
        Vertices[vertId + 2].texCoord * barycentrics.z;

    const float3 lightDirWS = float3(1.0f, 1.0f, 0.0f);

    // Calculate GI.
    const uint GIRayCount = 10;
    float3 gi = 0.0f;
    for (uint i = 0; i < GIRayCount; i++)
    {
        uint seed = initRand(payload.randomSeed * GIRayCount + i, FrameCount, 16);
        float3 direction = normalize(getCosHemisphereSample(seed, normalOS));
        gi += TraceGIRay(hitPosition, direction, payload.depth) / GIRayCount;
    }
    payload.color.rgb += gi * 0.4f;

    // Calculate AO.
    const uint aoRayCount = 10;
    float aoVal = 0.0f;
    for (uint i = 0; i < aoRayCount; i++)
    {
        uint seed = initRand(payload.randomSeed * aoRayCount + i, FrameCount, 16);
        float3 direction = normalize(getCosHemisphereSample(seed, normalOS));
        aoVal += TraceAORay(hitPosition, direction, payload.depth) / aoRayCount;
    }
    payload.color *= max(aoVal, 0.5f);
}

[shader("closesthit")]
void AOClosestHitShader(inout AORayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.aoVal = 0.0f;
}

[shader("closesthit")]
void GIClosestHitShader(inout GIRayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.depth += 1;
    float3 barycentrics = GetBarycentrics(attr.barycentrics);
    uint vertId = 3 * PrimitiveIndex() + Offsets[GeometryIndex()];

    float4 hitPosition = float4(HitWorldPosition(), 1.0f);
    float3 normalOS = Vertices[vertId + 0].normalOS * barycentrics.x +
        Vertices[vertId + 1].normalOS * barycentrics.y +
        Vertices[vertId + 2].normalOS * barycentrics.z;
    float2 uv = Vertices[vertId + 0].texCoord * barycentrics.x +
        Vertices[vertId + 1].texCoord * barycentrics.y +
        Vertices[vertId + 2].texCoord * barycentrics.z;

    float4 positionCS = mul(WorldToProjectionMatrix, hitPosition);
    float2 screenUV = (positionCS.xy / positionCS.w + 1.0f) / 2.0f;
    screenUV.y = 1 - screenUV.y;
    uint2 coord = screenUV * uint2(rcp(TAAJitter.zw));
    payload.color = Result[coord];

    // Calculate GI.
    const uint GIRayCount = 10 / payload.depth;
    float3 gi = 0.0f;
    for (uint i = 0; i < GIRayCount; i++)
    {
        uint seed = initRand(payload.randomSeed * GIRayCount + i, FrameCount, 16);
        float3 direction = normalize(getCosHemisphereSample(seed, normalOS));
        gi += TraceGIRay(hitPosition, direction, payload.depth) / GIRayCount;
    }
    payload.color.rgb += gi * 0.5f;
}

[shader("miss")]
void MissShader(inout RayPayload payload)
{
    payload.color = SkyboxCube.SampleLevel(SourceTextureSampler, payload.direction, 0.0f);
}

[shader("miss")]
void AOMissShader(inout AORayPayload payload)
{
    payload.aoVal = 1.0f;
}

[shader("miss")]
void GIMissShader(inout GIRayPayload payload)
{
    payload.color = SkyboxCube.SampleLevel(SourceTextureSampler, payload.direction, 0.0f);
}

#endif // RAYTRACING_HLSL
