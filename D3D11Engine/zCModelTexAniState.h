#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCModelTexAniState
{
public:
	void UpdateTexList()
	{
		XCALL(GothicMemoryLocations::zCModelTexAniState::UpdateTexList);
	}

	int	NumNodeTex;
	zCTexture** NodeTexList;
	int	ActAniFrames[8];
};