#ifndef FRUSTUM_CULLING_HLSL
#define FRUSTUM_CULLING_HLSL

#define HLSL

#include "Library/CommonRayTracing.hlsli"
#include "../../Sources/Shared/SharedPrimitives.h"
#include "../../Sources/Shared/SharedTypes.h"
#include "../../Sources/Shared/SharedConstants.h"

RaytracingAccelerationStructure Scene : register(t0);

[shader("raygeneration")]
void FrustumCullingRaygenShader()
{
    float3 origin, direction;
    GetRay(origin, direction);

    uint currentRayRecursionDepth = 0;
    //RayPayload payload = TraceRadianceRay(origin, normalize(world.xyz - origin), currentRayRecursionDepth);
    //Result[DispatchRaysIndex().xy] *= payload.attenuation;
    //Result[DispatchRaysIndex().xy] += payload.color;
}

#endif
