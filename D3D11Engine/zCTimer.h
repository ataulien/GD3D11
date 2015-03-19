#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCTimer {
public:
	/** Hooks the functions of this Class */
	static void Hook()
	{
/*#ifdef BUILD_GOTHIC_1_08k
		// There is a sleep in Gothic 1 which slows the whole game down here

		DWORD dwProtect;
		VirtualProtect((void *)0x005E0850, 0xCC, PAGE_EXECUTE_READWRITE, &dwProtect);

		// Replace this glorious sleep-call
		REPLACE_RANGE(0x005E089A, 0x005E08A1-1, INST_NOP);

#endif*/
	}

   float factorMotion;        //zREAL        //nicht zu klein machen. Sonst: Freeze bei hoher Framerate!
   float frameTimeFloat;      //zREAL [msec] //Zeit der zwischen diesem und dem letzten Frame verstrichen ist
   float totalTimeFloat;      //zREAL [msec] //gesamte Zeit
#ifndef BUILD_GOTHIC_1_08k
   float frameTimeFloatSecs;  //zREAL  [s]
   float totalTimeFloatSecs;  //zREAL  [s]
#endif
   DWORD lastTimer;           //zDWORD
   DWORD frameTime;           //zDWORD [msec] //nochmal als Ganzahl
   DWORD totalTime;           //zDWORD [msec]
   DWORD minFrameTime;        //zDWORD       //antifreeze. Sonst wird die Framezeit auf 0 gerundet und nichts bewegt sich
     
   DWORD forcedMaxFrameTime;  //zDWORD //länger als das darf ein Frame (in Spielzeit) nicht dauern. Um zu große Zeitsprünge für die Objekte zu vermeiden? Jedenfalls sort dies dafür, dass das Spiel langsamer läuft, wenn das Spiel mit rendern nicht hinterherkommt.

   static zCTimer* GetTimer(){return (zCTimer *)GothicMemoryLocations::GlobalObjects::zCTimer;}
};