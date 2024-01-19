#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL

#include "Library/CommonRayTracing.hlsli"
#include "Library/Random.hlsli"
#include "../../Sources/Shared/SharedPrimitives.h"
#include "../../Sources/Shared/SharedTypes.h"
#include "../../Sources/Shared/SharedConstants.h"

RaytracingAccelerationStructure Scene : register(t0);
RWTexture2D<float4> Result : register(u0);

StructuredBuffer<uint16_t> Indices : register(t1);
StructuredBuffer<Vertex> Vertices : register(t2);
StructuredBuffer<uint> Offsets : register(t3);

TextureCube SkyboxCube  : register(t10);
Texture2D DepthTexture : register(t11);
Texture2D ColorTexture : register(t12);

RayPayload TraceRadianceRay(float3 origin, float3 direction, in uint currentRayRecursionDepth)
{
    RayPayload payload =
    {
        float4(0, 0, 0, 0),
        direction,
        1.0f,
        currentRayRecursionDepth,
        DispatchRaysIndex().x + DispatchRaysIndex().y * DispatchRaysDimensions().x
    };

    if (currentRayRecursionDepth >= RaytracingConstants::kMaxRayRecursiveDepth)
    {
        return payload;
    }

    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000.0;

    uint flags = RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

    TraceRay(Scene, flags, 0xFF, RayType::Radiance, 0, RayType::Radiance, rayDesc, payload);

    return payload;
}

float TraceAORay(float3 origin, float3 direction, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= RaytracingConstants::kMaxRayRecursiveDepth)
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
    if (currentRayRecursionDepth >= RaytracingConstants::kMaxRayRecursiveDepth)
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

float TraceShadowRay(float3 origin, float3 direction, in uint currentRayRecursionDepth)
{
    if (currentRayRecursionDepth >= RaytracingConstants::kMaxRayRecursiveDepth)
    {
        return 0.2f;
    }
    
    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000.0;

    uint flags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
        | RAY_FLAG_CULL_BACK_FACING_TRIANGLES
        | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;

    ShadowRayPayload shadowRayPayload = { 0.2f };
    TraceRay(Scene, flags, 0xFF, RayType::Shadow, 0, RayType::Shadow, rayDesc, shadowRayPayload);

    return shadowRayPayload.attenuation;
}

[shader("raygeneration")]
void RaygenShader()
{
    float3 origin, direction;
    GetRay(origin, direction);

    uint currentRayRecursionDepth = 0;
    RayPayload payload = TraceRadianceRay(origin, direction, currentRayRecursionDepth);
    Result[DispatchRaysIndex().xy] *= payload.attenuation;
    Result[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void ClosestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.depth += 1;
    float3 barycentrics = GetBarycentrics(attr.barycentrics);
    uint vertId = 3 * PrimitiveIndex() + Offsets[GeometryIndex()];

    float3 hitPosition = HitWorldPosition();
    float3 normalOS = Vertices[Indices[vertId]].normalOS * barycentrics.x +
        Vertices[Indices[vertId + 1]].normalOS * barycentrics.y +
        Vertices[Indices[vertId + 2]].normalOS * barycentrics.z;

    const float3 lightDirWS = float3(1.0f, 1.0f, -1.0f);

    // Calculate GI.
    const uint GIRayCount = 10;
    float3 gi = 0.0f;
    uint i = 0;
    for (; i < GIRayCount; i++)
    {
        uint seed = InitRand(payload.randomSeed * GIRayCount + i, FrameCount, 16);
        float2 randVal = float2(NextRand(seed), NextRand(seed));
        float3 direction = normalize(GetCosHemisphereSample(randVal, normalOS));
        gi += TraceGIRay(hitPosition, direction, payload.depth) / GIRayCount;
    }
    payload.color.rgb += gi * 0.5f;

    // Calculate AO.
    const uint aoRayCount = 10;
    float aoVal = 0.0f;
    for (i = 0; i < aoRayCount; i++)
    {
        uint seed = InitRand(payload.randomSeed * aoRayCount + i, FrameCount, 16);
        float2 randVal = float2(NextRand(seed), NextRand(seed));
        float3 direction = normalize(GetCosHemisphereSample(randVal, normalOS));
        aoVal += TraceAORay(hitPosition, direction, payload.depth) / aoRayCount;
    }
    payload.color *= max(aoVal, 0.5f);

    // Calculate Shadow.
    float3 direction = normalize(float3(1.0f, 1.0f, -1.0f));
    payload.attenuation = TraceShadowRay(hitPosition, direction, payload.depth);
}

[shader("closesthit")]
void AOClosestHitShader(inout AORayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.aoVal = 0.0f;
}

[shader("closesthit")]
void ShadowClosestHitShader(inout ShadowRayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload.attenuation = 0.0f;
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

    float4 positionCS = mul(WorldToProjectionMatrix, hitPosition);
    float3 positionNDC = positionCS.xyz / positionCS.w;
    float2 screenUV = (positionNDC.xy + 1.0f) / 2.0f;
    screenUV.y = 1 - screenUV.y;
    float4 color = ColorTexture.SampleLevel(StaticLinearClampSampler, screenUV, 0.0f);
    float depth = DepthTexture.SampleLevel(StaticLinearClampSampler, screenUV, 0.0f).r;
    depth = max(depth + 0.0001f, 1.0f);
    float4 skybox = SkyboxCube.SampleLevel(StaticLinearClampSampler, payload.direction, 0.0f);
    payload.color = lerp(color, skybox, step(depth, positionNDC.z));

    // Calculate GI.
    const uint GIRayCount = 10 / payload.depth;
    float3 gi = 0.0f;
    for (uint i = 0; i < GIRayCount; i++)
    {
        uint seed = InitRand(payload.randomSeed * GIRayCount + i, FrameCount, 16);
        float2 randVal = float2(NextRand(seed), NextRand(seed));
        float3 direction = normalize(GetCosHemisphereSample(randVal, normalOS));
        gi += TraceGIRay(hitPosition.xyz, direction, payload.depth) / GIRayCount;
    }
    payload.color.rgb += gi * 0.5f;
}

[shader("miss")]
void MissShader(inout RayPayload payload)
{
    payload.color = SkyboxCube.SampleLevel(StaticLinearClampSampler, payload.direction, 0.0f);
}

[shader("miss")]
void ShadowMissShader(inout ShadowRayPayload payload)
{
    payload.attenuation = 1.0f;
}

[shader("miss")]
void AOMissShader(inout AORayPayload payload)
{
    payload.aoVal = 1.0f;
}

[shader("miss")]
void GIMissShader(inout GIRayPayload payload)
{
    payload.color = SkyboxCube.SampleLevel(StaticLinearClampSampler, payload.direction, 0.0f);
}

#endif // RAYTRACING_HLSL
