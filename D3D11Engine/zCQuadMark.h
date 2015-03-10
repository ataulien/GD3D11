#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zSTRING.h"

class zCQuadMark
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_zCQuadMarkCreateQuadMark = (zCQuadMarkCreateQuadMark)DetourFunction((BYTE *)GothicMemoryLocations::zCQuadMark::CreateQuadMark, (BYTE *)zCQuadMark::Hooked_CreateQuadMark);
		//HookedFunctions::OriginalFunctions.original_zCQuadMarkConstructor = (GenericThiscall)DetourFunction((BYTE *)GothicMemoryLocations::zCQuadMark::Constructor, (BYTE *)zCQuadMark::Hooked_Constructor);
		HookedFunctions::OriginalFunctions.original_zCQuadMarkDestructor = (GenericDestructor)DetourFunction((BYTE *)GothicMemoryLocations::zCQuadMark::Destructor, (BYTE *)zCQuadMark::Hooked_Destructor);
	}

	static void __fastcall Hooked_CreateQuadMark(void* thisptr, void* unknwn, zCPolygon* poly, const D3DXVECTOR3& position, const D3DXVECTOR2& size, struct zTEffectParams* params)
	{
		hook_infunc

		HookedFunctions::OriginalFunctions.original_zCQuadMarkCreateQuadMark(thisptr, poly, position, size, params);

		QuadMarkInfo* info = Engine::GAPI->GetQuadMarkInfo((zCQuadMark *)thisptr);

		WorldConverter::UpdateQuadMarkInfo(info, (zCQuadMark *)thisptr, position);

		if(!info->Mesh)
			Engine::GAPI->RemoveQuadMark((zCQuadMark *)thisptr);

		hook_outfunc
	}

	static void __fastcall Hooked_Constructor(void* thisptr, void* unknwn)
	{
		hook_infunc

		HookedFunctions::OriginalFunctions.original_zCQuadMarkConstructor(thisptr);

		hook_outfunc
	}

	static void __fastcall Hooked_Destructor(void* thisptr, void* unknwn)
	{
		hook_infunc

		HookedFunctions::OriginalFunctions.original_zCQuadMarkDestructor(thisptr);

		Engine::GAPI->RemoveQuadMark((zCQuadMark *)thisptr);

		hook_outfunc
	}

	zCMesh* GetQuadMesh()
	{
		return *(zCMesh**)THISPTR_OFFSET(GothicMemoryLocations::zCQuadMark::Offset_QuadMesh);
	}

	zCMaterial* GetMaterial()
	{
		return *(zCMaterial**)THISPTR_OFFSET(GothicMemoryLocations::zCQuadMark::Offset_Material);
	}

	zCVob* GetConnectedVob()
	{
		return *(zCVob **)THISPTR_OFFSET(GothicMemoryLocations::zCQuadMark::Offset_ConnectedVob);
	}
};