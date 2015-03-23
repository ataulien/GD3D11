#include "pch.h"
#include "IkarusBindings.h"
#include "Engine.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"
#include "GothicAPI.h"

#include "D3D11GraphicsEngine.h" // TODO: Needed for the UI-View. This should not be here!
#include "D2DView.h"

#include "zSTRING.h"
#include "zCParser.h"

extern "C"
{
	/** Draws a red cross at the given location in the current frame
		- Position: Pointer to the vector to draw the cross at
		- Size: Size of the cross. (About 25 is the size of a human head) */
	__declspec(dllexport) void __cdecl GDX_AddPointLocator(float3* position, float size)
	{
		Engine::GraphicsEngine->GetLineRenderer()->AddPointLocator(*position->toD3DXVECTOR3(), size, D3DXVECTOR4(1,0,0,1));
	}

	/** Sets the fog-color to use when not in fog-zone */
	__declspec(dllexport) void __cdecl GDX_SetFogColor(DWORD color)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.FogColorMod = float3(color);
	}

	/** Sets the global fog-density when not in fog-zone
		- Density: Very small values are needed, like 0.00004f for example. */
	__declspec(dllexport) void __cdecl GDX_SetFogDensity(float density)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.FogGlobalDensity = density;
	}

	/** Sets the height of the fog */
	__declspec(dllexport) void __cdecl GDX_SetFogHeight(float height)
{
		Engine::GAPI->GetRendererState()->RendererSettings.FogHeight = height;
	}

	/** Sets the falloff of the fog. A very small value means no falloff at all. */
	__declspec(dllexport) void __cdecl GDX_SetFogHeightFalloff(float falloff)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.FogHeightFalloff = falloff;
	}

	/** Sets the sun color */
	__declspec(dllexport) void __cdecl GDX_SetSunColor(DWORD color)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.SunLightColor = float3(color);
	}

	/** Sets the strength of the sun. Values above 1.0f are supported. */
	__declspec(dllexport) void __cdecl GDX_SetSunStrength(float strength)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.SunLightStrength = strength;
	}

	/** Sets base-strength of the dynamic shadows. 0 means no dynamic shadows are not visible at all. */
	__declspec(dllexport) void __cdecl GDX_SetShadowStrength(float strength)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.ShadowStrength = strength;
	}

	/** Sets strength of the original vertex lighting on the worldmesh for pixels which are in shadow.
		Keep in mind that these pixels will also be darkened by the ShadowStrength-Parameter*/
	__declspec(dllexport) void __cdecl GDX_SetShadowAOStrength(float strength)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.ShadowAOStrength = strength;
	}

	/** Sets strength of the original vertex lighting on the worldmesh for pixels which are NOT in shadow */
	__declspec(dllexport) void __cdecl GDX_SetWorldAOStrength(float strength)
	{
		Engine::GAPI->GetRendererState()->RendererSettings.WorldAOStrength = strength;
	}

	/** Callback for the messageboxes */
	static void MB_Callback(ED2D_MB_ACTION action, void* userdata)
	{
		int* id = (int *)userdata;

		// Call script-callback
		zCPARSER_CALL_FUNC(*id, action);

		delete id;
	}

	/** Opens a messagebox using the UI-Framework
		- Message: Text to display in the body of the message-box
		- Caption: Header of the message-box
		- Type: 0 = OK, 1 = YES/NO
		- Callback: Script-Function ID to use as a callback. */
	__declspec(dllexport) void __cdecl GDX_OpenMessageBox(zSTRING* message, zSTRING* caption, int type, int callbackID)
	{
		D3D11GraphicsEngine* g = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

		// Initialize the UI-Framework. Will do nothing if already done
		g->CreateMainUIView();

		// Check again, in case it failed
		if(g->GetUIView())
		{
			// Store the callback ID in memory
			int* d = new int;
			*d = callbackID;

			// Register the messagebox
			g->GetUIView()->AddMessageBox(caption->ToChar(), message->ToChar(), MB_Callback, d, (ED2D_MB_TYPE)type);
		}
	}
};