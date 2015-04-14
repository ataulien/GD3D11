#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCSoundSystem
{
public:

	void SetGlobalReverbPreset(int preset, float weight)
	{
		// Get vtable-entry
		int* vtbl = (int*)((int*)this)[0];

		typedef void(__thiscall* pFun)(void*,int, float);

		pFun fn = (pFun)vtbl[GothicMemoryLocations::zCSoundSystem::VTBL_SetGlobalReverbPreset];
		fn(this, preset, weight);
	}

	static zCSoundSystem* GetSoundSystem(){return *(zCSoundSystem **)GothicMemoryLocations::GlobalObjects::zSound;}
};