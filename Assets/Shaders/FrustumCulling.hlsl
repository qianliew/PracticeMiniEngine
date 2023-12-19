#ifndef FRUSTUM_CULLING_HLSL
#define FRUSTUM_CULLING_HLSL

#define HLSL

#include "Library/Common.hlsli"
#include "../../Sources/Shared/SharedPrimitives.h"
#include "../../Sources/Shared/SharedTypes.h"
#include "../../Sources/Shared/SharedConstants.h"

RaytracingAccelerationStructure Scene : register(t0);

[shader("raygeneration")]
void FrustumCullingRaygenShader()
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
    //RayPayload payload = TraceRadianceRay(origin, normalize(world.xyz - origin), currentRayRecursionDepth);
    //Result[DispatchRaysIndex().xy] *= payload.attenuation;
    //Result[DispatchRaysIndex().xy] += payload.color;
}

#endif
