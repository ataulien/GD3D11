#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCResourceManager.h"
#include "zSTRING.h"
#include "D3D7\MyDirectDrawSurface7.h"

class zCTexture
{
public:
	zCTexture(void);
	~zCTexture(void);

	/** Hooks the functions of this Class */
	static void Hook()
	{
		//HookedFunctions::OriginalFunctions.original_zCTex_D3DXTEX_BuildSurfaces = (zCTex_D3DXTEX_BuildSurfaces)DetourFunction((BYTE *)GothicMemoryLocations::zCTexture::XTEX_BuildSurfaces, (BYTE *)zCTexture::hooked_XTEX_BuildSurfaces);
		HookedFunctions::OriginalFunctions.ofiginal_zCTextureLoadResourceData = (zCTextureLoadResourceData)DetourFunction((BYTE *)GothicMemoryLocations::zCTexture::LoadResourceData, (BYTE *)zCTexture::hooked_LoadResourceData);
	
		
	}

	static int __fastcall hooked_LoadResourceData(void* thisptr)
	{
		Engine::GAPI->SetBoundTexture(7, (zCTexture *)thisptr); // Slot 7 is reserved for this
		int ret = HookedFunctions::OriginalFunctions.ofiginal_zCTextureLoadResourceData(thisptr);

		Engine::GAPI->SetBoundTexture(7, NULL); // Slot 7 is reserved for this

		/*if(ret)
		{
			zCTexture* tx = (zCTexture *)thisptr;
			MyDirectDrawSurface7* srf = tx->GetSurface();

			if(srf)
			{
				//LogInfo() << "Loading " << tx->GetNameWithoutExt();
				srf->LoadAdditionalResources(tx);
			}
		}*/

		return ret;
	}


	static int __fastcall hooked_XTEX_BuildSurfaces(void* thisptr, void* unknwn, int iVal)
	{
		// Notify the texture and load resources
		int ret = HookedFunctions::OriginalFunctions.original_zCTex_D3DXTEX_BuildSurfaces(thisptr, iVal);

		return ret;
	}

	const char* GetName()
	{
		return __GetName().ToChar();
	}

	std::string GetNameWithoutExt()
	{
		std::string n = GetName();

		int p = n.find_last_of('.');

		if(p != std::string::npos)
			n.resize(p);

		return n;
	}

	MyDirectDrawSurface7* GetSurface()
	{
		return *(MyDirectDrawSurface7 **)THISPTR_OFFSET(GothicMemoryLocations::zCTexture::Offset_Surface);
	}

	void Bind(int slot = 0)
	{
		Engine::GAPI->SetBoundTexture(slot, this);

		_Bind(0, slot);
	}

	void _Bind(bool unkwn, int stage = 0)
	{
		XCALL(GothicMemoryLocations::zCTexture::zCTex_D3DInsertTexture);
	}
	
	int LoadResourceData()
	{
		XCALL(GothicMemoryLocations::zCTexture::LoadResourceData);
	}

	zTResourceCacheState GetCacheState()
	{
		unsigned char state = *(unsigned char *)THISPTR_OFFSET(GothicMemoryLocations::zCTexture::Offset_CacheState);

		return (zTResourceCacheState)(state & GothicMemoryLocations::zCTexture::Mask_CacheState);
	}

	zTResourceCacheState CacheIn(float priority)
	{
		if (GetCacheState()==zRES_CACHED_IN)
		{
			TouchTimeStamp();
		} else 
		{
			Engine::GAPI->SetBoundTexture(7, this); // Index 7 is reserved for cacheIn
			TouchTimeStampLocal();
			zCResourceManager::GetResourceManager()->CacheIn(this, priority);
		}

		if(!GetSurface() || !GetSurface()->IsSurfaceReady())
			return zRES_CACHED_OUT;

		return GetCacheState();
	}

	void TouchTimeStamp()
	{
		XCALL(GothicMemoryLocations::zCTexture::zCResourceTouchTimeStamp);
	}

	void TouchTimeStampLocal()
	{
		XCALL(GothicMemoryLocations::zCTexture::zCResourceTouchTimeStampLocal);
	}

	bool HasAlphaChannel()
	{
		unsigned char flags = *(unsigned char *)THISPTR_OFFSET(GothicMemoryLocations::zCTexture::Offset_Flags);
		return (flags & GothicMemoryLocations::zCTexture::Mask_FlagHasAlpha) != 0;
	}

	/*void Release()
	{
		XCALL(GothicMemoryLocations::zCObject::Release);
	}*/

/*#ifdef BUILD_GOTHIC_1_08k
	zCTexture* GetAniTexture()
	{
		XCALL(GothicMemoryLocations::zCTexture::GetAniTexture);
	}
#endif*/

private:
	const zSTRING& __GetName()
	{
		XCALL(GothicMemoryLocations::zCObject::GetObjectName);
	}
};

