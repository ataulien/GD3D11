#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zSTRING.h"
#include "zCVisual.h"

struct DecalSettings
{
	zCMaterial* DecalMaterial;
	D3DXVECTOR2 DecalSize;
	D3DXVECTOR2 DecalOffset;
	BOOL DecalTwoSided;
	BOOL IgnoreDayLight;
	BOOL DecalOnTop;
};

class zCDecal : public zCVisual
{
public:

	DecalSettings* GetDecalSettings()
	{
		return (DecalSettings *)THISPTR_OFFSET(GothicMemoryLocations::zCDecal::Offset_DecalSettings);
	}

	bool GetAlphaTestEnabled()
	{
#ifdef BUILD_GOTHIC_1_08k
		return true;
#else
		XCALL(GothicMemoryLocations::zCDecal::GetAlphaTestEnabled);
#endif
	}
};