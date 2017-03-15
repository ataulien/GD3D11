#include "pch.h"

#include <D3D11.h>
#include <D3DX11.h>
#include "ddraw.h"
#include "d3d.h"
#include "D3D7/MyDirectDraw.h"
#include "Logger.h"
#include "detours.h"
#include "DbgHelp.h"
#include "BaseAntTweakBar.h"
#include "HookedFunctions.h"
#include <signal.h>
#include "VersionCheck.h"

#pragma comment(lib, "d3dx.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Imagehlp.lib")

//#pragma pack(1)

// Signal nvidia drivers that we want the high-performance card on laptops
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

static HINSTANCE hLThis = 0;
static HINSTANCE hDDRAW = 0;

static HRESULT (WINAPI *DirectDrawCreateEx_t)(GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter);
typedef void (WINAPI *DirectDrawSimple)();
typedef HRESULT (WINAPI *DirectDrawCreateEx_type)(GUID FAR*, LPVOID*, REFIID ,IUnknown FAR*);

void SignalHandler(int signal)
{
	LogInfo() << "Signal:" << signal;
	throw "!Access Violation!";
}

struct ddraw_dll
{
	HMODULE dll;
	FARPROC	AcquireDDThreadLock;
	FARPROC	CheckFullscreen;
	FARPROC	CompleteCreateSysmemSurface;
	FARPROC	D3DParseUnknownCommand;
	FARPROC	DDGetAttachedSurfaceLcl;
	FARPROC	DDInternalLock;
	FARPROC	DDInternalUnlock;
	FARPROC	DSoundHelp;
	FARPROC	DirectDrawCreate;
	FARPROC	DirectDrawCreateClipper;
	FARPROC	DirectDrawCreateEx;
	FARPROC	DirectDrawEnumerateA;
	FARPROC	DirectDrawEnumerateExA;
	FARPROC	DirectDrawEnumerateExW;
	FARPROC	DirectDrawEnumerateW;
	FARPROC	DllCanUnloadNow;
	FARPROC	DllGetClassObject;
	FARPROC	GetDDSurfaceLocal;
	FARPROC	GetOLEThunkData;
	FARPROC	GetSurfaceFromDC;
	FARPROC	RegisterSpecialCase;
	FARPROC	ReleaseDDThreadLock;
} ddraw;

HRESULT DoHookedDirectDrawCreateEx(GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter)
{
	*lplpDD = new MyDirectDraw(NULL);

	if(!Engine::GraphicsEngine)
	{		
		Engine::GAPI->OnGameStart();
		Engine::CreateGraphicsEngine();
	}

	return S_OK;
}

extern "C" HRESULT WINAPI HookedDirectDrawCreateEx(GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter) {
	//LogInfo() << "HookedDirectDrawCreateEx!";
	HRESULT hr = S_OK;

	if(Engine::PassThrough)
	{
		DirectDrawCreateEx_type fn = (DirectDrawCreateEx_type)ddraw.DirectDrawCreateEx;
		return fn(lpGuid, lplpDD, iid, pUnkOuter);
	}

	hook_infunc

		return DoHookedDirectDrawCreateEx(lpGuid, lplpDD, iid, pUnkOuter);

	hook_outfunc

		return hr;
}

extern "C" void WINAPI HookedAcquireDDThreadLock()
{
	if(Engine::PassThrough)
	{
		DirectDrawSimple fn = (DirectDrawSimple)ddraw.AcquireDDThreadLock;
		fn();
		return;
	}
	// Do nothing
	LogInfo() << "AcquireDDThreadLock called!";
}

extern "C" void WINAPI HookedReleaseDDThreadLock()
{
	if(Engine::PassThrough)
	{
		DirectDrawSimple fn = (DirectDrawSimple)ddraw.ReleaseDDThreadLock;
		fn();
		return;
	}

	// Do nothing
	LogInfo() << "ReleaseDDThreadLock called!";
}



__declspec(naked) void FakeAcquireDDThreadLock()			{ _asm { jmp [ddraw.AcquireDDThreadLock] } }
__declspec(naked) void FakeCheckFullscreen()				{ _asm { jmp [ddraw.CheckFullscreen] } }
__declspec(naked) void FakeCompleteCreateSysmemSurface()	{ _asm { jmp [ddraw.CompleteCreateSysmemSurface] } }
__declspec(naked) void FakeD3DParseUnknownCommand()			{ _asm { jmp [ddraw.D3DParseUnknownCommand] } }
__declspec(naked) void FakeDDGetAttachedSurfaceLcl()		{ _asm { jmp [ddraw.DDGetAttachedSurfaceLcl] } }
__declspec(naked) void FakeDDInternalLock()					{ _asm { jmp [ddraw.DDInternalLock] } }
__declspec(naked) void FakeDDInternalUnlock()				{ _asm { jmp [ddraw.DDInternalUnlock] } }
__declspec(naked) void FakeDSoundHelp()						{ _asm { jmp [ddraw.DSoundHelp] } }
// HRESULT WINAPI DirectDrawCreate( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
__declspec(naked) void FakeDirectDrawCreate()				{ _asm { jmp [ddraw.DirectDrawCreate] } }
// HRESULT WINAPI DirectDrawCreateClipper( DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter );
__declspec(naked) void FakeDirectDrawCreateClipper()		{ _asm { jmp [ddraw.DirectDrawCreateClipper] } }
// HRESULT WINAPI DirectDrawCreateEx( GUID FAR * lpGuid, LPVOID *lplpDD, REFIID iid,IUnknown FAR *pUnkOuter );
__declspec(naked) void FakeDirectDrawCreateEx()				{ _asm { jmp [ddraw.DirectDrawCreateEx] } }
// HRESULT WINAPI DirectDrawEnumerateA( LPDDENUMCALLBACKA lpCallback, LPVOID lpContext );
__declspec(naked) void FakeDirectDrawEnumerateA()			{ _asm { jmp [ddraw.DirectDrawEnumerateA] } }
// HRESULT WINAPI DirectDrawEnumerateExA( LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags );
__declspec(naked) void FakeDirectDrawEnumerateExA()			{ _asm { jmp [ddraw.DirectDrawEnumerateExA] } }
// HRESULT WINAPI DirectDrawEnumerateExW( LPDDENUMCALLBACKEXW lpCallback, LPVOID lpContext, DWORD dwFlags );
__declspec(naked) void FakeDirectDrawEnumerateExW()			{ _asm { jmp [ddraw.DirectDrawEnumerateExW] } }
// HRESULT WINAPI DirectDrawEnumerateW( LPDDENUMCALLBACKW lpCallback, LPVOID lpContext );
__declspec(naked) void FakeDirectDrawEnumerateW()			{ _asm { jmp [ddraw.DirectDrawEnumerateW] } }
__declspec(naked) void FakeDllCanUnloadNow()				{ _asm { jmp [ddraw.DllCanUnloadNow] } }
__declspec(naked) void FakeDllGetClassObject()				{ _asm { jmp [ddraw.DllGetClassObject] } }
__declspec(naked) void FakeGetDDSurfaceLocal()				{ _asm { jmp [ddraw.GetDDSurfaceLocal] } }
__declspec(naked) void FakeGetOLEThunkData()				{ _asm { jmp [ddraw.GetOLEThunkData] } }
__declspec(naked) void FakeGetSurfaceFromDC()				{ _asm { jmp [ddraw.GetSurfaceFromDC] } }
__declspec(naked) void FakeRegisterSpecialCase()			{ _asm { jmp [ddraw.RegisterSpecialCase] } }
__declspec(naked) void FakeReleaseDDThreadLock()			{ _asm { jmp [ddraw.ReleaseDDThreadLock] } }

void EnableCrashingOnCrashes()
{
	typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags);
	typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags);
	const DWORD EXCEPTION_SWALLOWING = 0x1;

	HMODULE kernel32 = LoadLibraryA("kernel32.dll");
	tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32,
		"GetProcessUserModeExceptionPolicy");
	tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32,
		"SetProcessUserModeExceptionPolicy");
	if (pGetPolicy && pSetPolicy)
	{
		DWORD dwFlags;
		if (pGetPolicy(&dwFlags))
		{
			// Turn off the filter
			pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
		}
	}
}


BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_ATTACH) {
		//DebugWrite_i("DDRAW Proxy DLL starting.\n", 0);
		hLThis = hInst;

		Engine::PassThrough = false;

		//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

		if(!Engine::PassThrough)
		{
			Log::Clear();
			LogInfo() << "Starting DDRAW Proxy DLL.";

			// Check for right version
			VersionCheck::CheckExecutable();

			HookedFunctions::OriginalFunctions.InitHooks();

			Engine::GAPI = nullptr;
			Engine::GraphicsEngine = nullptr;

			// Create GothicAPI here to make all hooks work
			Engine::CreateGothicAPI();

			GothicAPI::DisableErrorMessageBroadcast();
			Engine::LoadDLLFiles();

			EnableCrashingOnCrashes();
			//SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
		}

		char infoBuf[MAX_PATH];
		GetSystemDirectoryA(infoBuf, MAX_PATH);
		// We then append \ddraw.dll, which makes the string:
		// C:\windows\system32\ddraw.dll
		strcat_s(infoBuf, MAX_PATH, "\\ddraw.dll");

		ddraw.dll = LoadLibraryA(infoBuf);     

		ddraw.AcquireDDThreadLock			= GetProcAddress(ddraw.dll, "AcquireDDThreadLock");
		ddraw.CheckFullscreen				= GetProcAddress(ddraw.dll, "CheckFullscreen");
		ddraw.CompleteCreateSysmemSurface	= GetProcAddress(ddraw.dll, "CompleteCreateSysmemSurface");
		ddraw.D3DParseUnknownCommand		= GetProcAddress(ddraw.dll, "D3DParseUnknownCommand");
		ddraw.DDGetAttachedSurfaceLcl		= GetProcAddress(ddraw.dll, "DDGetAttachedSurfaceLcl");
		ddraw.DDInternalLock				= GetProcAddress(ddraw.dll, "DDInternalLock");
		ddraw.DDInternalUnlock				= GetProcAddress(ddraw.dll, "DDInternalUnlock");
		ddraw.DSoundHelp					= GetProcAddress(ddraw.dll, "DSoundHelp");
		ddraw.DirectDrawCreate				= GetProcAddress(ddraw.dll, "DirectDrawCreate");
		ddraw.DirectDrawCreateClipper		= GetProcAddress(ddraw.dll, "DirectDrawCreateClipper");
		ddraw.DirectDrawCreateEx			= GetProcAddress(ddraw.dll, "DirectDrawCreateEx");
		ddraw.DirectDrawEnumerateA			= GetProcAddress(ddraw.dll, "DirectDrawEnumerateA");
		ddraw.DirectDrawEnumerateExA		= GetProcAddress(ddraw.dll, "DirectDrawEnumerateExA");
		ddraw.DirectDrawEnumerateExW		= GetProcAddress(ddraw.dll, "DirectDrawEnumerateExW");
		ddraw.DirectDrawEnumerateW			= GetProcAddress(ddraw.dll, "DirectDrawEnumerateW");
		ddraw.DllCanUnloadNow				= GetProcAddress(ddraw.dll, "DllCanUnloadNow");
		ddraw.DllGetClassObject				= GetProcAddress(ddraw.dll, "DllGetClassObject");
		ddraw.GetDDSurfaceLocal				= GetProcAddress(ddraw.dll, "GetDDSurfaceLocal");
		ddraw.GetOLEThunkData				= GetProcAddress(ddraw.dll, "GetOLEThunkData");
		ddraw.GetSurfaceFromDC				= GetProcAddress(ddraw.dll, "GetSurfaceFromDC");
		ddraw.RegisterSpecialCase			= GetProcAddress(ddraw.dll, "RegisterSpecialCase");
		ddraw.ReleaseDDThreadLock			= GetProcAddress(ddraw.dll, "ReleaseDDThreadLock");

		*(void**)&DirectDrawCreateEx_t = (void*)GetProcAddress(ddraw.dll, "DirectDrawCreateEx");
	} else if (reason == DLL_PROCESS_DETACH) {
		FreeLibrary(hDDRAW);

		Engine::OnShutDown();

		LogInfo() << "DDRAW Proxy DLL signing off.\n";
	}
	return TRUE;
};