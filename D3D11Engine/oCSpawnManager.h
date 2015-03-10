#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"
#include "oCGame.h"
#include "zCVob.h"

class oCSpawnManager
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_oCSpawnManagerSpawnNpc = (oCSpawnManagerSpawnNpc)DetourFunction((BYTE *)GothicMemoryLocations::oCSpawnManager::SpawnNpc, (BYTE *)oCSpawnManager::hooked_oCSpawnManagerSpawnNpc);
	}

	/** Reads config stuff */
	static void __fastcall hooked_oCSpawnManagerSpawnNpc(void* thisptr, void* unknwn, class oCNpc* npc, const D3DXVECTOR3& position, float f)
	{
		hook_infunc
		HookedFunctions::OriginalFunctions.original_oCSpawnManagerSpawnNpc(thisptr, npc, position, f);

		Engine::GAPI->OnRemovedVob((zCVob *)npc, ((zCVob *)npc)->GetHomeWorld());	
		Engine::GAPI->OnAddVob((zCVob *)npc, ((zCVob *)npc)->GetHomeWorld());
		hook_outfunc
	}
};



