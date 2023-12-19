#pragma once

class AABBBox
{
private:
    D3D12_RAYTRACING_AABB aabb;

public:
    AABBBox() = delete;
    AABBBox(D3D12_RAYTRACING_AABB inAABB);
    ~AABBBox();

    inline const D3D12_RAYTRACING_AABB GetData() const { return aabb; }
};
