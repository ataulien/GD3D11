#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCVob.h"

class oCNPC : public zCVob
{
public:
	oCNPC(void);
	~oCNPC(void);

	/** Hooks the functions of this Class */
	static void Hook()
	{
#ifdef BUILD_GOTHIC_1_08k
		// Only G1 needs this, because newly added NPCs only get enabled, but not re-added to the world like in G2
		//HookedFunctions::OriginalFunctions.original_oCNPCEnable = (oCNPCEnable)DetourFunction((BYTE *)GothicMemoryLocations::oCNPC::Enable, (BYTE *)oCNPC::hooked_oCNPCEnable);
#endif

		HookedFunctions::OriginalFunctions.original_oCNPCInitModel = (GenericThiscall)DetourFunction((BYTE *)GothicMemoryLocations::oCNPC::InitModel, (BYTE *)oCNPC::hooked_oCNPCInitModel);
	}

	static void __fastcall hooked_oCNPCInitModel(void* thisptr, void* unknwn)
	{
		hook_infunc	
		HookedFunctions::OriginalFunctions.original_oCNPCInitModel(thisptr);

		if(Engine::GAPI->GetSkeletalVobByVob((zCVob *)thisptr))
		{
			// This may causes the vob to be added and removed multiple times, but makes sure we get all changes of armor
			Engine::GAPI->OnRemovedVob((zCVob *)thisptr, ((zCVob *)thisptr)->GetHomeWorld());	
			Engine::GAPI->OnAddVob((zCVob *)thisptr, ((zCVob *)thisptr)->GetHomeWorld());
		}
		hook_outfunc
	}

	/** Reads config stuff */
	static void __fastcall hooked_oCNPCEnable(void* thisptr, void* unknwn, D3DXVECTOR3& position)
	{
		hook_infunc
		HookedFunctions::OriginalFunctions.original_oCNPCEnable(thisptr, position);

		Engine::GAPI->OnRemovedVob((zCVob *)thisptr, ((zCVob *)thisptr)->GetHomeWorld());	
		Engine::GAPI->OnAddVob((zCVob *)thisptr, ((zCVob *)thisptr)->GetHomeWorld());
		hook_outfunc
	}

	void ResetPos(const D3DXVECTOR3& pos)
	{
		XCALL(GothicMemoryLocations::oCNPC::ResetPos);
	}
};

