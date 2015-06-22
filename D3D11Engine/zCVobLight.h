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

	void DoAnimation()
	{
		XCALL(GothicMemoryLocations::zCVobLight::DoAnimation);
	}

	bool IsStatic()
	{
		int flags = *(int *)THISPTR_OFFSET(GothicMemoryLocations::zCVobLight::Offset_IsStatic);
		return (flags & 1) != 0;
	}

#ifndef PUBLIC_RELEASE
	byte data1[0x140];
	float flt[10];
#endif
};