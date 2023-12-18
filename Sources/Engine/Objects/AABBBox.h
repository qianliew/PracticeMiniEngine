#pragma once

class AABBBox
{
private:
    XMFLOAT3 min;
    XMFLOAT3 max;

public:
    AABBBox();
    ~AABBBox();
};
