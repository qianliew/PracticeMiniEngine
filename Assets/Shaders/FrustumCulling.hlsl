#ifndef FRUSTUM_CULLING_HLSL
#define FRUSTUM_CULLING_HLSL

#define HLSL

#include "Library/CommonRayTracing.hlsli"
#include "../../Sources/Shared/SharedPrimitives.h"
#include "../../Sources/Shared/SharedTypes.h"
#include "../../Sources/Shared/SharedConstants.h"

RaytracingAccelerationStructure Scene : register(t0);
RWTexture2D<float4> Result : register(u0);

float TraceFrustumCullingRay(float3 origin, float3 direction, in uint currentRayRecursionDepth)
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

    uint flags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH
        | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
    TraceRay(Scene, flags, 0xFF, 0, 0, 0, rayDesc, payload);

    return payload.vis;
}

[shader("raygeneration")]
void FrustumCullingRaygenShader()
{
    float3 origin, direction;
    GetRay(origin, direction);

    uint currentRayRecursionDepth = 0;
    float vis = TraceFrustumCullingRay(origin, direction, currentRayRecursionDepth);
    Result[DispatchRaysIndex().xy] = vis;
}

[shader("miss")]
void FrustumCullingMissShader(inout FrustumCullingRayPayload payload)
{
    payload.vis = 1.0f;
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
    // if (RayAnalyticGeometryIntersectionTest(ray, primitiveType, thit, attr))
    {
        ReportHit(thit, /*hitKind*/ 0, attr);
    }
}

#endif
