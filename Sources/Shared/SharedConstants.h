#pragma once

#ifndef SHARED_CONSTANTS_H
#define SHARED_CONSTANTS_H

namespace RaytracingConstants
{
	static const UINT MaxRayRecursiveDepth = 2;
}

namespace RayType
{
	enum Enum
	{
		Radiance = 0,
		AO = 1,
		Count = 2,
	};
};

#endif // !SHARED_CONSTANTS_H
