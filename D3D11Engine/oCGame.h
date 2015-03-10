#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCVob.h"

class zCWorld;
class zView;

#define GOTHIC_KEY_F1 0x3B
#define GOTHIC_KEY_F2 0x3C
#define GOTHIC_KEY_F3 0x3D
#define GOTHIC_KEY_F4 0x3E
#define GOTHIC_KEY_F5 0x3F
#define GOTHIC_KEY_F6 0x40
#define GOTHIC_KEY_F7 0x41
#define GOTHIC_KEY_F8 0x42
#define GOTHIC_KEY_F9 0x43
#define GOTHIC_KEY_F10 0x44

class oCNPC;
class oCGame
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_oCGameEnterWorld = (oCGameEnterWorld)DetourFunction((BYTE *)GothicMemoryLocations::oCGame::EnterWorld, (BYTE *)oCGame::hooked_EnterWorld);
	}

	static void __fastcall hooked_EnterWorld(void* thisptr, void* unknwn, oCNpc* playerVob, int changePlayerPos, const zSTRING& startpoint)
	{
		HookedFunctions::OriginalFunctions.original_oCGameEnterWorld(thisptr, playerVob, changePlayerPos, startpoint);
		Engine::GAPI->OnWorldLoaded();
	}

	void TestKey(int key)
	{
		XCALL(GothicMemoryLocations::oCGame::TestKeys);
	}

	static oCNPC* GetPlayer()
	{
		return *(oCNPC **)GothicMemoryLocations::oCGame::Var_Player;
	}

    int _vtbl;
    int _zCSession_csMan;        //zCCSManager*
    zCWorld* _zCSession_world;        //zCWorld*
    int _zCSession_camera;       //zCCamera*
    int _zCSession_aiCam;        //zCAICamera*
    zCVob* _zCSession_camVob;       //zCVob*
    zView* _zCSession_viewport;     //zCView*

	static oCGame* GetGame() { return *(oCGame**)GothicMemoryLocations::GlobalObjects::oCGame; };
};