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
#ifdef BUILD_GOTHIC_1_08k
		return false; // FIXME
#endif

		zSTRING* zCmdline = (zSTRING *)THISPTR_OFFSET(GothicMemoryLocations::zCOption::Offset_CommandLine);
		std::string cmdLine = zCmdline->ToChar();
		std::string cmd = str;

		// Make them uppercase
		std::transform(cmdLine.begin(), cmdLine.end(),cmdLine.begin(), ::toupper);
		std::transform(cmd.begin(), cmd.end(),cmd.begin(), ::toupper);

		return cmdLine.find("-" + cmd) != std::string::npos;
	}

	/** Reads config stuff */
	static int __fastcall hooked_zOptionReadBool(void* thisptr, void* unknwn,zSTRING const& section, char const* var, int def)
	{
		
		int r = 0;
		if(_stricmp(var, "zWaterAniEnabled") == 0)
		{
			Engine::GAPI->SetIntParamFromConfig("zWaterAniEnabled", 0);
			return 0; // Disable water animations
		}else if(_stricmp(var, "scaleVideos") == 0) // Force scaleVideos to get them into the upper left corner
		{
			Engine::GAPI->SetIntParamFromConfig("scaleVideos", 0);
			return 0;
		}/*else if(stricmp(var, "zStartupWindowed") == 0)
		{
			Engine::GAPI->SetIntParamFromConfig(1);
			return 1;
		}*/

		

		r = HookedFunctions::OriginalFunctions.original_zCOptionReadBool(thisptr, section, var, def);
		Engine::GAPI->SetIntParamFromConfig(var, r);
		return r;
	}

	/** Reads config stuff */
	static long __fastcall Do_hooked_zOptionReadInt(void* thisptr, zSTRING const& section, char const* var, int def)
	{
		BaseGraphicsEngine* engine = Engine::GraphicsEngine;
		LogInfo() << "Reading Gothic-Config: " << var;

		if(_stricmp(var, "zVidResFullscreenX") == 0)
		{
			LogInfo() << "Forcing zVidResFullscreenX: " << engine->GetResolution().x;
			return engine->GetResolution().x;
		}else if(_stricmp(var, "zVidResFullscreenY") == 0)
		{
			LogInfo() << "Forcing zVidResFullscreenY: " << engine->GetResolution().y;
			return engine->GetResolution().y;
		}else if(_stricmp(var, "zVidResFullscreenBPP") == 0)
		{
			return 32;
		}else if(_stricmp(var, "zTexMaxSize") == 0)
		{
			return 16384;
		}else if(_stricmp(var, "zTexCacheSizeMaxBytes") == 0)
		{
			return INT_MAX;
		}

		return HookedFunctions::OriginalFunctions.original_zCOptionReadInt(thisptr, section, var, def);
	}

	static long __fastcall hooked_zOptionReadInt(void* thisptr, void* unknwn,zSTRING const& section, char const* var, int def)
	{		
		int i = Do_hooked_zOptionReadInt(thisptr,section, var, def);

		// Save the variable
		Engine::GAPI->SetIntParamFromConfig("var", i);

		return i;
	}

	static zCOption* GetOptions(){return *(zCOption**)GothicMemoryLocations::GlobalObjects::zCOption;}
};



