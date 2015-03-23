#pragma once
#include "pch.h"

class zSTRING;
extern "C"
{
	__declspec(dllexport) void __cdecl GD3D11_AddPointLocator(float3* position, float size);
	
	__declspec(dllexport) void __cdecl GD3D11_SetFogColor(DWORD color);
	__declspec(dllexport) void __cdecl GD3D11_SetFogDensity(float density);

	__declspec(dllexport) void __cdecl GD3D11_OpenMessageBox(zSTRING* message, zSTRING* caption, int type, int callbackID);
};