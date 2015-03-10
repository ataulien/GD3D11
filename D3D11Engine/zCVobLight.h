#pragma once
#include "zCVob.h"

class zCVobLight : public zCVob
{
public:

	DWORD GetLightColor()
	{
		return *(DWORD *)THISPTR_OFFSET(GothicMemoryLocations::zCVobLight::Offset_LightColor);
	}

	float GetLightRange()
	{
		return *(float *)THISPTR_OFFSET(GothicMemoryLocations::zCVobLight::Offset_Range);
	}

	bool IsEnabled()
	{
		return ((*(DWORD *)THISPTR_OFFSET(GothicMemoryLocations::zCVobLight::Offset_LightInfo)) & GothicMemoryLocations::zCVobLight::Mask_LightEnabled) != 0;
	}
};