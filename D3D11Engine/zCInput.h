#pragma once
#include "pch.h"
#include "HookedFunctions.h"

class zCInput
{
public:
	int GetDeviceEnabled(int dev)
	{
		XCALL(GothicMemoryLocations::zCInput::GetDeviceEnabled);
	}

	void SetDeviceEnabled(int dev, int i)
	{
		XCALL(GothicMemoryLocations::zCInput::SetDeviceEnabled);
	}

	inline static zCInput* GetInput()
	{ 
		return *(zCInput**)GothicMemoryLocations::GlobalObjects::zCInput;
	};

};