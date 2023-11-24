#pragma once

#ifndef SHARED_CONSTANTS_H
#define SHARED_CONSTANTS_H

namespace RaytracingConstants
{
	static const UINT MaxRayRecursiveDepth = 5;
}

namespace RayType
{
	enum Enum
	{
		Radiance = 0,
		AO = 1,
		GI = 2,
		Count = 3,
	};
};

#endif // !SHARED_CONSTANTS_H
