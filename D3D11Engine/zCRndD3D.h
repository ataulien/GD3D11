#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCRndD3D
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		//HookedFunctions::OriginalFunctions.original_zCRnd_D3DVid_SetScreenMode = (zCRnd_D3DVid_SetScreenMode)DetourFunction((BYTE *)GothicMemoryLocations::zCRndD3D::VidSetScreenMode, (BYTE *)zCRndD3D::hooked_zCRndD3DVidSetScreenMode);
		DetourFunction((BYTE *)GothicMemoryLocations::zCRndD3D::DrawLineZ, (BYTE *)hooked_zCRndD3DDrawLineZ);
	}

	/** Overwritten to only accept windowed */
	static void __fastcall hooked_zCRndD3DVidSetScreenMode(void* thisptr, void* unknwn, int mode)
	{
		hook_infunc
		// Pass Windowed only.
		HookedFunctions::OriginalFunctions.original_zCRnd_D3DVid_SetScreenMode(thisptr, 1);

		hook_outfunc
	}

	/** Overwritten to only accept windowed */
	static void __fastcall hooked_zCRndD3DDrawLineZ(void* thisptr, void* unknwn, float x1, float y1, float z1, float x2, float y2, float z2, DWORD color)
	{
		// Do nothing here yet.
		// TODO: Implement!
	}


};