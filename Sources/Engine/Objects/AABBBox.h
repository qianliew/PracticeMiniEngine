#pragma once

class AABBBox
{
private:
    XMFLOAT3 minPos;
    XMFLOAT3 maxPos;

public:
    AABBBox() = delete;
    AABBBox(XMFLOAT3 inMinPos, XMFLOAT3 inMaxPos);
    ~AABBBox();
};
