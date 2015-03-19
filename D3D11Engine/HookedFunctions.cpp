#include "pch.h"
#include "HookedFunctions.h"

#include "zCBspTree.h"
#include "zCWorld.h"
#include "oCGame.h"
#include "zCMaterial.h"
#include "zFILE.h"
#include "zCOption.h"
#include "zCRndD3D.h"
#include "zCParticleFX.h"
#include "zCView.h"
#include "CGameManager.h"
#include "zCVisual.h"
#include "zCTimer.h"
#include "zCModel.h"
#include "oCSpawnManager.h"
#include "zCVob.h"
#include "zCTexture.h"
#include "zCThread.h"
#include "zCResourceManager.h"
#include "zCQuadMark.h"
#include "oCNPC.h"

#if _MSC_VER >= 1300
#include <Tlhelp32.h>
#endif

#include "StackWalker.h"



/** Init all hooks here */
void HookedFunctionInfo::InitHooks()
{
	LogInfo() << "Initializing hooks";

	DWORD dwProtect;
	VirtualProtect((void *)GothicMemoryLocations::zCWorld::Render, 0x255, PAGE_EXECUTE_READWRITE, &dwProtect);

	oCGame::Hook();
	zCBspTree::Hook();
	zCWorld::Hook();
	zCMaterial::Hook();
	zCBspNode::Hook();
	zFILE::Hook();
	zCOption::Hook();
	zCRndD3D::Hook();
	zCParticleFX::Hook();
	zCView::Hook();
	CGameManager::Hook();
	zCVisual::Hook();
	zCTimer::Hook();
	zCModel::Hook();
	oCSpawnManager::Hook();
	zCVob::Hook();
	zCTexture::Hook();
	zCThread::Hook();
	//zCResourceManager::Hook();
	zCQuadMark::Hook();
	oCNPC::Hook();

	//original_zCExceptionHandler_UnhandledExceptionFilter = (zCExceptionHandlerUnhandledExceptionFilter)DetourFunction((BYTE *)GothicMemoryLocations::Functions::zCExceptionHandler_UnhandledExceptionFilter, (BYTE *)HookedFunctionInfo::hooked_zCExceptionHandlerUnhandledExceptionFilter);
	//original_HandledWinMain = (HandledWinMain)DetourFunction((BYTE *)GothicMemoryLocations::Functions::HandledWinMain, (BYTE *)HookedFunctionInfo::hooked_HandledWinMain);
	//original_ExitGameFunc = (ExitGameFunc)DetourFunction((BYTE *)GothicMemoryLocations::Functions::ExitGameFunc, (BYTE *)HookedFunctionInfo::hooked_ExitGameFunc);

	// Hook the single bink-function
	DetourFunction((BYTE *)GothicMemoryLocations::zCBinkPlayer::GetPixelFormat, (BYTE *)HookedFunctionInfo::hooked_zBinkPlayerGetPixelFormat);

	original_zCBinkPlayerOpenVideo = (zCBinkPlayerOpenVideo)DetourFunction((BYTE *)GothicMemoryLocations::zCBinkPlayer::OpenVideo, (BYTE *)HookedFunctionInfo::hooked_zBinkPlayerOpenVideo);

	original_Alg_Rotation3DNRad = (Alg_Rotation3DNRad)GothicMemoryLocations::Functions::Alg_Rotation3DNRad;
}

/** Function hooks */
int __stdcall HookedFunctionInfo::hooked_HandledWinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR szCmdLine, int sw)
{

	int r = HookedFunctions::OriginalFunctions.original_HandledWinMain(hInstance, hPrev, szCmdLine, sw);

	return r;
}

void __cdecl HookedFunctionInfo::hooked_ExitGameFunc()
{
	Engine::OnShutDown();

	HookedFunctions::OriginalFunctions.hooked_ExitGameFunc();
}

long __stdcall HookedFunctionInfo::hooked_zCExceptionHandlerUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionPtrs)
{
	return HookedFunctions::OriginalFunctions.original_zCExceptionHandler_UnhandledExceptionFilter(exceptionPtrs);
}

/** Returns the pixelformat of a bink-surface */
long __fastcall HookedFunctionInfo::hooked_zBinkPlayerGetPixelFormat(void* thisptr, void* unknwn, zTRndSurfaceDesc& desc)
{
	int* cd = (int *)&desc;

	// Resolution is at pos [2] and [3]
	//cd[2] = Engine::GraphicsEngine->GetResolution().x;
	//cd[3] = Engine::GraphicsEngine->GetResolution().y;

	/*for(int i=0;i<0x7C;i++)
	{
	cd[i] = i;
	}*/

	return 4; // 4 satisfies gothic enough to play the video
	//Global::HookedFunctions.zBinkPlayerGetPixelFormat(thisptr, desc);
}

int __fastcall HookedFunctionInfo::hooked_zBinkPlayerOpenVideo(void* thisptr, void* unknwn, zSTRING str)
{
	int r = HookedFunctions::OriginalFunctions.original_zCBinkPlayerOpenVideo(thisptr, str);

	struct BinkInfo
	{
		unsigned int ResX;
		unsigned int ResY;
		// ... unimportant
	};

	// Grab the resolution
	// This structure stores width and height as first two parameters, as ints.
	BinkInfo* res = *(BinkInfo **)(((char *)thisptr) + (GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle));

	if(res)
	{
		Engine::GAPI->GetRendererState()->RendererInfo.PlayingMovieResolution = INT2(res->ResX, res->ResY);
	}

	return r;
}