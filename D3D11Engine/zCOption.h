#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"
#include <algorithm>

class zCOption
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_zCOptionReadInt = (zCOptionReadInt)DetourFunction((BYTE *)GothicMemoryLocations::zCOption::ReadInt, (BYTE *)zCOption::hooked_zOptionReadInt);
		HookedFunctions::OriginalFunctions.original_zCOptionReadBool = (zCOptionReadInt)DetourFunction((BYTE *)GothicMemoryLocations::zCOption::ReadBool, (BYTE *)zCOption::hooked_zOptionReadBool);

	}

	/** Returns true if the given string is in the commandline of the game */
	bool IsParameter(const std::string& str)
	{
		zSTRING* zCmdline = (zSTRING *)THISPTR_OFFSET(GothicMemoryLocations::zCOption::Offset_CommandLine);
		std::string cmdLine = zCmdline->ToChar();
		std::string cmd = str;

		// Make them uppercase
		std::transform(cmdLine.begin(), cmdLine.end(),cmdLine.begin(), ::toupper);
		std::transform(cmd.begin(), cmd.end(),cmd.begin(), ::toupper);

		return cmd.find("-" + cmd) != std::string::npos;
	}

	/** Reads config stuff */
	static int __fastcall hooked_zOptionReadBool(void* thisptr, void* unknwn,zSTRING const& section, char const* var, int def)
	{
		hook_infunc
		if(strcmp(var, "zWaterAniEnabled") == 0)
		{
			return 0; // Disable water animations
		}else if(strcmp(var, "zStartupWindowed") == 0)
		{
			return 1;
		}

		return HookedFunctions::OriginalFunctions.original_zCOptionReadBool(thisptr, section, var, def);
		hook_outfunc

		return 0;
	}

	/** Reads config stuff */
	static long __fastcall Do_hooked_zOptionReadInt(void* thisptr, zSTRING const& section, char const* var, int def)
	{
		BaseGraphicsEngine* engine = Engine::GraphicsEngine;
		LogInfo() << "Reading Gothic-Config: " << var;

		if(strcmp(var, "zVidResFullscreenX") == 0)
		{
			LogInfo() << "Forcing zVidResFullscreenX: " << engine->GetResolution().x;
			return engine->GetResolution().x;
		}else if(strcmp(var, "zVidResFullscreenY") == 0)
		{
			LogInfo() << "Forcing zVidResFullscreenY: " << engine->GetResolution().y;
			return engine->GetResolution().y;
		}else if(strcmp(var, "zVidResFullscreenBPP") == 0)
		{
			return 32;
		}else if(strcmp(var, "zTexMaxSize") == 0)
		{
			return 16384;
		}else if(strcmp(var, "zTexCacheSizeMaxBytes") == 0)
		{
			return INT_MAX;
		}

		return HookedFunctions::OriginalFunctions.original_zCOptionReadInt(thisptr, section, var, def);
	}

	static long __fastcall hooked_zOptionReadInt(void* thisptr, void* unknwn,zSTRING const& section, char const* var, int def)
	{
		hook_infunc
		
		return Do_hooked_zOptionReadInt(thisptr,section, var, def);

		hook_outfunc

		return 0;
	}

	static zCOption* GetOptions(){return *(zCOption**)GothicMemoryLocations::GlobalObjects::zCOption;}
};



