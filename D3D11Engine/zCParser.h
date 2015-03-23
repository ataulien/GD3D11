#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zSTRING.h"



class zCParser 
{
public:
	static void CallFunc(zCParser* p, int symbolId, ...)
	{
		__asm { mov eax, GothicMemoryLocations::zCParser::CallFunc  };
        __asm { jmp eax };
		//XCALL(GothicMemoryLocations::zCParser::CallFunc);
	}

	static zCParser* GetParser(){return (zCParser *)GothicMemoryLocations::GlobalObjects::zCParser;}
};