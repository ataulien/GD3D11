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

	enum EVisualType 
	{
		VT_OTHER,
		VT_PROGMESHPROTO,
		VT_MODEL,
		VT_PARTICLE_FX,
		VT_MORPHMESH,
		VT_DECAL,
	};

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

	/** File extension this visual uses. Handy for finding out what class this is */
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

	/** Returns the class-type of this visual */
	EVisualType GetVisualType()
	{
		std::vector<std::string> extv;
	
		int e=0;
		while(strlen(GetFileExtension(e)) > 0)
		{
			extv.push_back(GetFileExtension(e));
			e++;
		}

		for(unsigned int i=0; i<extv.size();i++)
		{
			std::string ext = extv[i];

			if(ext == ".3DS")
				return VT_PROGMESHPROTO;
			else if(ext == ".MMS")
				return VT_MORPHMESH;
			else if(ext == ".MDS" || ext == ".ASC")
				return VT_MODEL;
			else if(ext == ".PFX")
				return VT_PARTICLE_FX;
			else if(ext == ".TGA")
				return VT_DECAL;
		}
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