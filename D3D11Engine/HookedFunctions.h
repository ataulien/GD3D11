#pragma once
#include "pch.h"
#include "detours.h"
#include "GothicMemoryLocations.h"
#include "zTypes.h"
#include "HookExceptionFilter.h"

/** This file stores the original versions of the hooked functions and the function declerations */

class zCFileBIN;
class zCCamera;
class zCVob;
class zSTRING;
class zCBspBase;
class oCNpc;
class zCPolygon;

template <class T> 
class zCTree;
class zCVisual;

typedef int (__thiscall* zCBspTreeLoadBIN)(void*,zCFileBIN&, int);
typedef void (__thiscall* zCWorldRender)(void*,zCCamera&);
typedef void (__thiscall* zCWorldVobAddedToWorld)(void*, zCVob*);
typedef void (__thiscall* zCWorldInsertVobInWorld)(void*, zCVob*);
typedef void (__thiscall* zCBspTreeAddVob)(void*, zCVob*);
typedef void (__thiscall* zCWorldLoadWorld)(void*, const zSTRING& fileName, const int loadMode);
typedef void (__thiscall* oCGameEnterWorld)(void*, oCNpc* playerVob, int changePlayerPos, const zSTRING& startpoint);
typedef void (__thiscall* zCWorldVobRemovedFromWorld)(void*, zCVob*);
typedef D3DXMATRIX (__cdecl* Alg_Rotation3DNRad)(const D3DXVECTOR3& axis, const float angle);
typedef int (__cdecl* vidGetFPSRate)(void);
typedef void (__thiscall* GenericDestructor)(void *);
typedef void (__thiscall* GenericThiscall)(void *);
typedef void (__thiscall* zCMaterialConstruktor)(void *);
typedef void (__thiscall* zCMaterialInitValues)(void *);
typedef void (__fastcall* zCBspNodeRenderIndoor)(void *, int);
typedef void (__fastcall* zCBspNodeRenderOutdoor)(void *, zCBspBase*, zTBBox3D, int, int);
typedef int (__thiscall* zFILEOpen)(void*,zSTRING&, bool);
typedef void (__thiscall* zCRnd_D3DVid_SetScreenMode)(void*, int);
typedef int (__thiscall* zCOptionReadInt)(void*,zSTRING const&, char const*, int);
typedef int (__thiscall* zCOptionReadBool)(void*,zSTRING const&, char const*, int);
typedef void (__cdecl* zCViewSetMode)(int, int, int, HWND*);
typedef int (__stdcall* HandledWinMain)(HINSTANCE, HINSTANCE, LPSTR, int);
typedef int (__thiscall* CGameManagerExitGame)(void*);
typedef const zSTRING* (__thiscall* zCVisualGetFileExtension)(void*, int);
typedef long (__stdcall* zCExceptionHandlerUnhandledExceptionFilter)(void*);
typedef void (__thiscall* zCWorldDisposeVobs)(void *, zCTree<zCVob> *);
typedef void (__thiscall* oCSpawnManagerSpawnNpc)(void*, class oCNpc *, const D3DXVECTOR3&, float);
typedef void (__thiscall* zCVobSetVisual)(void*, zCVisual*);
typedef int (__thiscall* zCTex_D3DXTEX_BuildSurfaces)(void*, int);
typedef int (__thiscall* zCTextureLoadResourceData)(void*);
typedef int (__thiscall* zCThreadSuspendThread)(void*);
typedef void (__thiscall* zCResourceManagerCacheOut)(void*,class zCResource*);
typedef void (__thiscall* zCQuadMarkCreateQuadMark)(void*, zCPolygon*, const D3DXVECTOR3&, const D3DXVECTOR2&, struct zTEffectParams*);

struct zTRndSurfaceDesc;
struct HookedFunctionInfo
{
	/** Init all hooks here */
	void InitHooks();

	zCBspTreeLoadBIN original_zCBspTreeLoadBIN;
	zCWorldRender original_zCWorldRender;
	zCWorldVobAddedToWorld original_zCWorldVobAddedToWorld;
	zCBspTreeAddVob original_zCBspTreeAddVob;
	zCWorldInsertVobInWorld original_zCWorldInsertVobInWorld;
	zCWorldLoadWorld original_zCWorldLoadWorld;
	oCGameEnterWorld original_oCGameEnterWorld;
	zCWorldVobRemovedFromWorld original_zCWorldVobRemovedFromWorld;
	Alg_Rotation3DNRad original_Alg_Rotation3DNRad;
	GenericDestructor original_zCMaterialDestructor;
	GenericDestructor original_zCParticleFXDestructor;
	GenericDestructor original_zCVisualDestructor;
	zCMaterialConstruktor original_zCMaterialConstruktor;
	zCMaterialInitValues original_zCMaterialInitValues;
	zFILEOpen original_zFILEOpen;
	zCRnd_D3DVid_SetScreenMode original_zCRnd_D3DVid_SetScreenMode;
	zCOptionReadInt original_zCOptionReadInt;
	zCOptionReadBool original_zCOptionReadBool;
	zCViewSetMode original_zCViewSetMode;
	HandledWinMain original_HandledWinMain;
	CGameManagerExitGame original_CGameManagerExitGame;
	zCExceptionHandlerUnhandledExceptionFilter original_zCExceptionHandler_UnhandledExceptionFilter;
	GenericThiscall original_zCWorldDisposeWorld;
	zCWorldDisposeVobs original_zCWorldDisposeVobs;
	oCSpawnManagerSpawnNpc original_oCSpawnManagerSpawnNpc;
	zCVobSetVisual original_zCVobSetVisual;
	zCTex_D3DXTEX_BuildSurfaces original_zCTex_D3DXTEX_BuildSurfaces;
	zCTextureLoadResourceData ofiginal_zCTextureLoadResourceData;
	zCThreadSuspendThread original_zCThreadSuspendThread;
	zCResourceManagerCacheOut original_zCResourceManagerCacheOut;
	zCQuadMarkCreateQuadMark original_zCQuadMarkCreateQuadMark;
	GenericDestructor original_zCQuadMarkDestructor;
	GenericThiscall original_zCQuadMarkConstructor;

	/** Function hooks */
	static int __stdcall hooked_HandledWinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR szCmdLine, int sw);
	static void __cdecl hooked_ExitGameFunc();

	/** Unhandled exception handler */
	static long __stdcall hooked_zCExceptionHandlerUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionPtrs);

	/** Single function for making the bink-player working again */
	/** Returns the pixelformat of a bink-surface */
	static long __fastcall hooked_zBinkPlayerGetPixelFormat(void* thisptr, void* unknwn, zTRndSurfaceDesc& desc);
};

namespace HookedFunctions
{
	/** Holds all the original functions */
	__declspec( selectany ) HookedFunctionInfo OriginalFunctions;
};