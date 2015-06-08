#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
//#include "zCWorld.h"
#include "zCObject.h"

class zCSkyPlanet
{
public:
	int vtbl;
	void* mesh; // 0
	D3DXVECTOR4 color0; // 4 
	D3DXVECTOR4 color1; // 20
	float		size; // 36
	D3DXVECTOR3 pos; // 40
	D3DXVECTOR3 rotAxis; // 52


};

enum zESkyLayerMode 
{ 
	zSKY_LAYER_MODE_POLY, 
	zSKY_LAYER_MODE_BOX 
};

class zCSkyLayerData 
{
public:
	zESkyLayerMode SkyMode;
	zCTexture* Tex;
	char zSTring_TexName[20];
	
	float TexAlpha;
	float TexScale;
	D3DXVECTOR2	TexSpeed;
};


class zCSkyState 
{
public:
	float Time;
	D3DXVECTOR3	PolyColor;
	D3DXVECTOR3	FogColor;
	D3DXVECTOR3	DomeColor1;
	D3DXVECTOR3	DomeColor0;
	float FogDist;
	int	SunOn;
	int	CloudShadowOn;
	zCSkyLayerData	Layer[2];
};

enum zTWeather
{
	zTWEATHER_SNOW,
	zTWEATHER_RAIN
};

class zCSkyLayer;

typedef void (__thiscall* zCSkyControllerRenderSkyPre)(void*);
typedef void (__thiscall* zCSkyControllerRenderSkyPost)(void*, int);
class zCSkyController : public zCObject
{
public:
	/*void RenderSkyPre()
	{
		int* vtbl = (int*)((int*)this)[0];

		zCSkyControllerRenderSkyPre fn = (zCSkyControllerRenderSkyPre)vtbl[GothicMemoryLocations::zCSkyController::VTBL_RenderSkyPre];
		fn(this);
	}

	void RenderSkyPost()
	{
		int* vtbl = (int*)((int*)this)[0];

		zCSkyControllerRenderSkyPost fn = (zCSkyControllerRenderSkyPost)vtbl[GothicMemoryLocations::zCSkyController::VTBL_RenderSkyPost];
		fn(this, 1);
	}*/

	DWORD* PolyLightCLUTPtr;
	float cloudShadowScale;
	BOOL ColorChanged;
	zTWeather Weather;
};
 
// Controler - Typo in Gothic
class zCSkyController_Outdoor : public zCSkyController
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		// Overwrite the rain-renderfunction and particle-updates
		DWORD dwProtect;
		VirtualProtect((void *)GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPStart, 
			GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPEnd 
			- GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPStart, 
			PAGE_EXECUTE_READWRITE, &dwProtect);

		REPLACE_RANGE(GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPStart, GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPEnd - 1, INST_NOP);
	}

	/** Updates the rain-weight and sound-effects */
	void ProcessRainFX()
	{
		XCALL(GothicMemoryLocations::zCSkyController_Outdoor::ProcessRainFX);
	}

	/** Returns the rain-fx weight */
	float GetRainFXWeight()
	{
		return *(float *)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_OutdoorRainFXWeight);
	}

	/*zCSkyPlanet* GetSun()
	{
		return *(zCSkyPlanet **)(((char *)this) + GothicMemoryLocations::zCSkyController_Outdoor::Offset_Sun);
	}*/

	/** Returns the continous master time, not wrapped between 0 and 1 */
	/*float GetMasterTime()
	{
		static float s_contMasterTime = 0.0f;
		static float s_masterTimeLast = 0.0f;

		// Add the delta of this frame so the time won't be wrapped between 0 and 1
		if(GetMasterTimeReal() < s_masterTimeLast)
			s_contMasterTime += GetMasterTimeReal() - (s_masterTimeLast - 1.0f); // Make sure we still get the actual delta and dont wrap back
		else
			s_contMasterTime += GetMasterTimeReal() - s_masterTimeLast;

		s_masterTimeLast = GetMasterTimeReal();

		return s_contMasterTime;
	}*/

	/** Returns the master-time wrapped between 0 and 1 */
	float GetMasterTime()
	{
		return *(float*)(((char *)this) + GothicMemoryLocations::zCSkyController_Outdoor::Offset_MasterTime);
	}

	int GetUnderwaterFX()
	{
#ifndef BUILD_GOTHIC_1_08k
		XCALL(GothicMemoryLocations::zCSkyController_Outdoor::GetUnderwaterFX);
#else
		return 0;
#endif
	}

	D3DXVECTOR3 GetOverrideColor()
	{
#ifndef BUILD_GOTHIC_1_08k
		return *(D3DXVECTOR3*)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_OverrideColor);
#else
		return D3DXVECTOR3(0,0,0);
#endif
	}

	bool GetOverrideFlag()
	{
#ifndef BUILD_GOTHIC_1_08k
		int f = *(int *)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_OverrideFlag);

		return f != 0;
#else
		return 0;
#endif
	}

	/** Returns the sun position in world coords */
	D3DXVECTOR3 GetSunWorldPosition(float timeScale = 1.0f)
	{
		/*if(!GetSun())
		{
			return D3DXVECTOR3(0,0,0);
		}*/

		//float angle = GetMasterTime() * 2.0f * (float)D3DX_PI; // Get mastertime into rad, 0 and 12 are now at the horizon, 18 is in the sky
		//angle += (float)D3DX_PI * 0.5f; // 12 is now in the sky, 18 horizon



		float angle = ((GetMasterTime() * timeScale - 0.3f) * 1.25f + 0.5f) * 2.0f * (180.0f * (float)D3DX_PI / 180.0f);

		D3DXVECTOR3 sunPos = D3DXVECTOR3(-60, 0, 100);
		D3DXVec3Normalize(&sunPos, &sunPos);

		D3DXVECTOR3 rotAxis = D3DXVECTOR3(1,0,0);

		D3DXMATRIX r = HookedFunctions::OriginalFunctions.original_Alg_Rotation3DNRad(rotAxis, -angle);
		D3DXMatrixTranspose(&r, &r);

		D3DXVECTOR3 pos;
		D3DXVec3TransformNormal(&pos, &sunPos, &r);

		return pos;
	}

	void SetCameraLocationHint(int hint)
	{
		XCALL(GothicMemoryLocations::zCSkyController_Outdoor::SetCameraLocationHint);
	}


	/*zCSkyLayer* GetSkyLayers(int i)
	{
		if(i==0)
			return (zCSkyLayer*)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_SkyLayer1);
		else
			return (zCSkyLayer*)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_SkyLayer2);
	}

	zCSkyState** GetSkyLayerStates()
	{
		return *(zCSkyState***)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_SkyLayerState);
		
	}

	zCSkyState* GetSkyState(int i)
	{
		if(i==0)
			return *(zCSkyState**)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_SkyLayerState0);
		else
			return *(zCSkyState**)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_SkyLayerState1);
	}*/

	zCSkyState* GetMasterState()
	{
		return (zCSkyState*)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_MasterState);
	}

	/*void Init()
	{
		XCALL(GothicMemoryLocations::zCSkyController_Outdoor::Init);
	}

	bool GetInitDone()
	{
		return (*(int *)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_InitDone)) != 0;
	}

	void Activate()
	{
		(*(zCSkyController_Outdoor **)GothicMemoryLocations::zCSkyController_Outdoor::OBJ_ActivezCSkyController) = this;
	}

	void Interpolate()
	{
		XCALL(GothicMemoryLocations::zCSkyController_Outdoor::Interpolate);
	}

	static zCSkyController_Outdoor* GetActiveSkyController()
	{
		return (*(zCSkyController_Outdoor **)GothicMemoryLocations::zCSkyController_Outdoor::OBJ_ActivezCSkyController);
	}*/
};

class zCMesh;
class zCSkyLayer 
{
public:
	zCMesh* SkyPolyMesh;
	zCPolygon* SkyPoly;
	D3DXVECTOR2 SkyTexOffs;
	zCMesh*	SkyDomeMesh;
	zESkyLayerMode SkyMode;

	/*bool IsNight()
	{
		zCSkyController_Outdoor* sky = oCGame::GetGame()->_zCSession_world->GetSkyControllerOutdoor();

		return this == sky->GetSkyLayers(0) && sky->GetMasterTime() >= 0.25f && sky->GetMasterTime() <= 0.75f;
	}

	int GetLayerChannel()
	{
		zCSkyController_Outdoor* sky = oCGame::GetGame()->_zCSession_world->GetSkyControllerOutdoor();

		return this == sky->GetSkyLayers(1) ? 1 : 0;
	}

	float3 GetSkyColor()
	{
		zCSkyController_Outdoor* sky = oCGame::GetGame()->_zCSession_world->GetSkyControllerOutdoor();

		D3DXVECTOR3 c;

		if(IsNight())
			c = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
		else
			c = sky->GetMasterState()->DomeColor1;

		if((sky->GetMasterTime() >= 0.35f) && (sky->GetMasterTime() <= 0.65f) && GetLayerChannel() == 1)
			c = 0.5f * (c + D3DXVECTOR3(1.0f, 1.0f, 1.0f));

		return float3(c);
	}*/
};