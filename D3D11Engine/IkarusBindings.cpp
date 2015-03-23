#include "pch.h"
#include "IkarusBindings.h"
#include "Engine.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"
#include "GothicAPI.h"

#include "D3D11GraphicsEngine.h"
#include "D2DView.h"

#include "zSTRING.h"
#include "zCParser.h"

extern "C"
{
	__declspec(dllexport) void __cdecl GD3D11_AddPointLocator(float3* position, float size)
	{
		Engine::GraphicsEngine->GetLineRenderer()->AddPointLocator(*position->toD3DXVECTOR3(), size, D3DXVECTOR4(1,0,0,1));
	}

	__declspec(dllexport) void __cdecl GD3D11_SetFogColor(DWORD color)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.FogColorMod = float3(color);
	}

	__declspec(dllexport) void __cdecl GD3D11_SetFogDensity(float density)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.FogGlobalDensity = density;
	}

	static void MB_Callback(ED2D_MB_ACTION action, void* userdata)
	{
		int* id = (int *)userdata;

		zCParser* parser = zCParser::GetParser();
		((void (*)(zCParser*, ...))GothicMemoryLocations::zCParser::CallFunc)(parser, *id);

		//zCParser::CallFunc(parser, *id);

		delete id;
	}

	__declspec(dllexport) void __cdecl GD3D11_OpenMessageBox(zSTRING* message, zSTRING* caption, int type, int callbackID)
	{
		D3D11GraphicsEngine* g = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

		int* d = new int;
		*d = callbackID;
		if(g->GetUIView())
			g->GetUIView()->AddMessageBox(caption->ToChar(), message->ToChar(), MB_Callback, d, (ED2D_MB_TYPE)type);
	}
};