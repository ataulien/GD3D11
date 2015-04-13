#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCTexture.h"

const int zMAT_GROUP_WATER = 5;


const int zMAT_ALPHA_FUNC_FUNC_MAT_DEFAULT = 0;
const int zMAT_ALPHA_FUNC_FUNC_NONE = 1;
const int zMAT_ALPHA_FUNC_BLEND = 2;
const int zMAT_ALPHA_FUNC_ADD = 3;
const int zMAT_ALPHA_FUNC_TEST = 7;
const int zMAT_ALPHA_FUNC_BLEND_TEST = 8;

class zCTexAniCtrl 
{
private:
	int	AniChannel;
	float ActFrame;
	float AniFPS;
	DWORD FrameCtr;
	int	IsOneShotAni;
};

class zCMaterial
{
public:
	zCMaterial(void);
	~zCMaterial(void);

	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_zCMaterialDestructor = (GenericDestructor)DetourFunction((BYTE *)GothicMemoryLocations::zCMaterial::Destructor, (BYTE *)zCMaterial::Hooked_Destructor);
		//HookedFunctions::OriginalFunctions.original_zCMaterialConstruktor = (zCMaterialConstruktor)DetourFunction((BYTE *)GothicMemoryLocations::zCMaterial::Constructor, (BYTE *)zCMaterial::Hooked_Constructor);
		HookedFunctions::OriginalFunctions.original_zCMaterialInitValues = (zCMaterialInitValues)DetourFunction((BYTE *)GothicMemoryLocations::zCMaterial::InitValues, (BYTE *)zCMaterial::Hooked_InitValues);

	}

	static void __fastcall Hooked_Destructor(void* thisptr, void* unknwn)
	{
		hook_infunc

		// Notify the world
		if(Engine::GAPI)
			Engine::GAPI->OnMaterialDeleted((zCMaterial *)thisptr);

		HookedFunctions::OriginalFunctions.original_zCMaterialDestructor(thisptr);

		hook_outfunc
	}

	static void __fastcall Hooked_Constructor(void* thisptr, void* unknwn)
	{
		hook_infunc
		// Notify the world
		//Engine::GAPI->OnMaterialCreated((zCMaterial *)thisptr);

		HookedFunctions::OriginalFunctions.original_zCMaterialConstruktor(thisptr);

		hook_outfunc
	}

	static void __fastcall Hooked_InitValues(void* thisptr, void* unknwn)
	{
		hook_infunc

		// Notify the world
		if(Engine::GAPI)
			Engine::GAPI->OnMaterialCreated((zCMaterial *)thisptr);

		HookedFunctions::OriginalFunctions.original_zCMaterialInitValues(thisptr);

		hook_outfunc
	}

	zCTexAniCtrl* GetTexAniCtrl()
	{
		return (zCTexAniCtrl *)(((char *)this) + GothicMemoryLocations::zCMaterial::Offset_TexAniCtrl);
	}

	/** Returns AniTexture */
	zCTexture* GetTexture()
	{		
		//return GetTextureSingle(); // FIXME: GetAniTexture crashes sometimes
		XCALL(GothicMemoryLocations::zCMaterial::GetAniTexture);
	}

	/** Returns single texture, because not all seem to be animated and returned by GetAniTexture? */
	zCTexture* GetTextureSingle()
	{
		return *(zCTexture **)(((char *)this) + GothicMemoryLocations::zCMaterial::Offset_Texture);
	}

	/** Returns the current texture from GetAniTexture */
	zCTexture* GetAniTexture()
	{
		XCALL(GothicMemoryLocations::zCMaterial::GetAniTexture);
	}

	void BindTexture(int slot)
	{
		if(GetTexture())
		{
			// Bind it
			if(GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
				GetTexture()->Bind(slot);
		}
	}

	void BindTextureSingle(int slot)
	{
		if(GetTextureSingle())
		{
			// Bind it
			if(GetTextureSingle()->CacheIn(0.6f) == zRES_CACHED_IN	)
				GetTextureSingle()->Bind(slot);
		}
	}

	int GetAlphaFunc()
	{
		return (*(int *)THISPTR_OFFSET(GothicMemoryLocations::zCMaterial::Offset_AlphaFunc)) & 0xFF;
	}

	void SetAlphaFunc(int func)
	{
		int f = (*(int *)THISPTR_OFFSET(GothicMemoryLocations::zCMaterial::Offset_AlphaFunc));
		f &= ~0xFF;

		f |= func;

		 (*(int *)THISPTR_OFFSET(GothicMemoryLocations::zCMaterial::Offset_AlphaFunc)) = f;
	}

	int GetMatGroup()
	{
		return *(int *)THISPTR_OFFSET(GothicMemoryLocations::zCMaterial::Offset_MatGroup);
	}

	bool HasAlphaTest()
	{
		int f = GetAlphaFunc();
		return f == zMAT_ALPHA_FUNC_TEST || f == zMAT_ALPHA_FUNC_BLEND_TEST;
	}
};

