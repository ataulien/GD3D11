#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zSTRING.h"

class zCVisual
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_zCVisualDestructor = (GenericDestructor)DetourFunction((BYTE *)GothicMemoryLocations::zCVisual::Destructor, (BYTE *)zCVisual::Hooked_Destructor);
	}

	static void __fastcall Hooked_Destructor(void* thisptr, void* unknwn)
	{
		hook_infunc
		// Notify the world
		if(Engine::GAPI)
			Engine::GAPI->OnVisualDeleted((zCVisual *)thisptr);

		HookedFunctions::OriginalFunctions.original_zCVisualDestructor(thisptr);

		hook_outfunc
	}

	const char* GetFileExtension(int i)
	{
		if(__GetFileExtension(i))
			return __GetFileExtension(i)->ToChar();

		return "";
	}

	const char* GetObjectName()
	{
		return __GetObjectName().ToChar();
	}

private:
	zSTRING& __GetObjectName()
	{
		XCALL(GothicMemoryLocations::zCObject::GetObjectName);
	}

	const zSTRING* __GetFileExtension(int i)
	{
		int* vtbl = (int*)((int*)this)[0];

		zCVisualGetFileExtension fn = (zCVisualGetFileExtension)vtbl[GothicMemoryLocations::zCVisual::VTBL_GetFileExtension];
		return fn(this, i);

	}
};