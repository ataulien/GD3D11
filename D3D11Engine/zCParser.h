#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zSTRING.h"



class zCParser 
{
public:
	static void CallFunc(int symbolId, ...)
	{
		va_list vl;

		zCParser* parser = zCParser::GetParser();
		((void (*)(zCParser*, ...))GothicMemoryLocations::zCParser::CallFunc)(parser, symbolId, vl);
	}

	static zCParser* GetParser(){return (zCParser *)GothicMemoryLocations::GlobalObjects::zCParser;}
};