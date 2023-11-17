//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL


#define HLSL
#include "RaytracingHlslCompat.h"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ConstantBuffer<RayGenConstantBuffer> g_rayGenCB : register(b0);

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
    float4 color;
};

bool IsInsideViewport(float2 p, Viewport viewport)
{
    return (p.x >= viewport.left && p.x <= viewport.right)
        && (p.y >= viewport.top && p.y <= viewport.bottom);
}

[shader("raygeneration")]
void MyRaygenShader()
{
    int count = 5;
    float aoVal = 0.0f;
    float3 origin = g_rayGenCB.cameraPosition.xyz;

    uint flags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH |
        RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;

    for (int i = -count; i < count + 1; i++)
    {
        for (int j = -count; j < count + 1; j++)
        {
            float2 xy = DispatchRaysIndex().xy + 0.5f; // center in the middle of the pixel.
            xy.x += i;
            xy.y += j;
            float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

            // Invert Y for DirectX-style coordinates.
            screenPos.y = -screenPos.y;

            // Unproject the pixel coordinate into a world positon.
            float4 world = mul(g_rayGenCB.projectionToWorld, float4(screenPos, 0, 1));
            world.xyz /= world.w;

            // Trace the ray.
            // Set the ray's extents.
            RayDesc ray;
            ray.Origin = origin;
            ray.Direction = normalize(world.xyz - origin);
            // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
            // TMin should be kept small to prevent missing geometry at close contact areas.
            ray.TMin = 0.001;
            ray.TMax = 50.0;
            RayPayload payload = { float4(0, 0, 0, 0) };

            TraceRay(Scene, flags, 0xFF, 0, 0, 0, ray, payload);
            aoVal += payload.color;
        }
    }

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = aoVal / (count * 2 + 1) / (count * 2 + 1);
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = attr.barycentrics.xyxy;
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(1, 1, 1, 1);
}

#endif // RAYTRACING_HLSL