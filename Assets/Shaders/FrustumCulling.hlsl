#ifndef FRUSTUM_CULLING_HLSL
#define FRUSTUM_CULLING_HLSL

#define HLSL

#include "Library/CommonRayTracing.hlsli"
#include "../../Sources/Shared/SharedPrimitives.h"
#include "../../Sources/Shared/SharedTypes.h"
#include "../../Sources/Shared/SharedConstants.h"

RaytracingAccelerationStructure Scene : register(t0);
RWTexture2D<float4> Result : register(u0);
RWStructuredBuffer<uint> VisData : register(u10);

float TraceFrustumCullingRay(float3 origin, float3 direction)
{
    FrustumCullingRayPayload payload =
    {
        0.0f,
    };

    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 10000.0;

    uint flags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    TraceRay(Scene, flags, 0xFF, 0, 0, 0, rayDesc, payload);

    return payload.vis;
}

[shader("raygeneration")]
void FrustumCullingRaygenShader()
{
    float3 origin, direction;
    GetRay(origin, direction);

    float vis = TraceFrustumCullingRay(origin, direction);
    Result[DispatchRaysIndex().xy] = vis;
}

[shader("miss")]
void FrustumCullingMissShader(inout FrustumCullingRayPayload payload)
{
    payload.vis = 1.0f;
}

[shader("closesthit")]
void FrustumCullingClosestHitShader(inout FrustumCullingRayPayload payload, in AABBAttributes attr)
{
    uint index = GeometryIndex() >> 5;
    uint bitIndex = GeometryIndex() % 32;
    VisData[index] = VisData[index] | (1 << bitIndex);
    payload.vis = GeometryIndex() / 3.0f + 0.1f;
}

Ray GetRayInAABBPrimitiveLocalSpace()
{
    Ray ray;
    ray.origin = ObjectRayOrigin();
    ray.direction = ObjectRayDirection();
    return ray;
}

[shader("intersection")]
void FrustumCullingIntersectionShader()
{
    Ray ray = GetRayInAABBPrimitiveLocalSpace();

    float thit = 5.0f;
    AABBAttributes attr = (AABBAttributes)0;
    ReportHit(thit, /*hitKind*/ 0, attr);
}

#endif
