#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"
#include "zCResourceManager.h"

class zCThread
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_zCThreadSuspendThread = (zCThreadSuspendThread)DetourFunction((BYTE *)GothicMemoryLocations::zCThread::SuspendThread, (BYTE *)zCThread::hooked_SuspendThread);

		//ThreadSleeping = false;
	}

	/** Reads config stuff */
	static int __fastcall hooked_SuspendThread(void* thisptr, void* unknwn)
	{
		//hook_infunc
		//Engine::GAPI->EnterResourceCriticalSection(); // Protect the game from running into a deadlock
		//Sleep(0);
		//Engine::GAPI->LeaveResourceCriticalSection();

		/*if(!zCResourceManager::GetResourceManagerMutex().try_lock())
		{
			LogInfo() << "Trying to suspend res-thread while doing work! This would result in a deadlock.";

			zCResourceManager::GetResourceManagerMutex().lock();
		}

		Sleep(0);
		zCResourceManager::GetResourceManagerMutex().unlock();*/

		/*ThreadSleeping = true;
		int r = HookedFunctions::OriginalFunctions.original_zCThreadSuspendThread(thisptr);
		ThreadSleeping = false;
		
		hook_outfunc

		return r;*/

		zCThread* t = (zCThread *)thisptr;
		int* suspCount = t->GetSuspendCounter();

		if((*suspCount) > 0)
			return 0;

		(*suspCount) += 1;

		while((*suspCount))
			Sleep(100); // Sleep as long as we are suspended

		return 1;
	}

	int* GetSuspendCounter()
	{
		return (int *)THISPTR_OFFSET(GothicMemoryLocations::zCThread::Offset_SuspendCount);
	}
};
