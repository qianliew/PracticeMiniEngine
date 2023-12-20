#pragma once

#ifndef SHARED_CONSTANTS_H
#define SHARED_CONSTANTS_H

namespace GlobalConstants
{
	static const UINT kMaxNumObject = 1024;
	static const UINT kVisDataSize = kMaxNumObject / 32;
}

namespace RaytracingConstants
{
	static const UINT kMaxRayRecursiveDepth = 3;
}

namespace GeometryType
{
	enum Type
	{
		Triangle = 0,
		AABB,
		Count
	};
}

namespace RayType
{
	enum Enum
	{
		Radiance = 0,
		AO = 1,
		GI = 2,
		Shadow = 3,
		Count = 4,
	};
};

#endif // !SHARED_CONSTANTS_H
