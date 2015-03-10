#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCTexture;

enum zTResourceCacheState {
	zRES_FAILURE	=-1,
	zRES_CACHED_OUT	= 0,
	zRES_QUEUED		= 1,
	zRES_LOADING	= 2,
	zRES_CACHED_IN	= 3
};

class zCResourceManager
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		//HookedFunctions::OriginalFunctions.original_zCResourceManagerCacheOut = (zCResourceManagerCacheOut)DetourFunction((BYTE *)GothicMemoryLocations::zCResourceManager::CacheOut, (BYTE *)zCResourceManager::hooked_CacheOut);
	}

	static void __fastcall hooked_CacheOut(void* thisptr, void* unknwn, class zCResource* res)
	{
		hook_infunc
		//Engine::GAPI->EnterResourceCriticalSection(); // Protect the game from running into a deadlock
		//Sleep(0);
		//Engine::GAPI->LeaveResourceCriticalSection();

		//GetResourceManagerMutex().lock();
		
		HookedFunctions::OriginalFunctions.original_zCResourceManagerCacheOut(thisptr, res);
		
		//GetResourceManagerMutex().unlock();

		hook_outfunc
	}

	zTResourceCacheState CacheIn(zCTexture *res, float priority)
	{
		XCALL(GothicMemoryLocations::zCResourceManager::CacheIn);
	}

	static std::mutex& GetResourceManagerMutex()
	{
		static std::mutex mutex;
		return mutex;
	}

	static zCResourceManager* GetResourceManager(){return *(zCResourceManager **)GothicMemoryLocations::GlobalObjects::zCResourceManager;}
};