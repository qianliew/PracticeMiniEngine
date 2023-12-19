#include "stdafx.h"
#include "AABBBox.h"

AABBBox::AABBBox(XMFLOAT3 inMinPos, XMFLOAT3 inMaxPos) :
	minPos(inMaxPos), maxPos(inMaxPos)
{

}

AABBBox::~AABBBox()
{

}
