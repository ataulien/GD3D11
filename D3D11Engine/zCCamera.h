#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"

enum zTCam_ClipType 
{ 
	ZTCAM_CLIPTYPE_IN, 
	ZTCAM_CLIPTYPE_OUT, 
	ZTCAM_CLIPTYPE_CROSSING 
};

class zCCamera
{
public:

	enum ETransformType
	{
		TT_WORLD,
		TT_VIEW,
		TT_WORLDVIEW,
		TT_WORLDVIEW_INV,
		TT_VIEW_INV
	};

	D3DXMATRIX const& GetTransform(const ETransformType type)
	{
		XCALL(GothicMemoryLocations::zCCamera::GetTransform);
	}

	void SetTransform(const ETransformType type, const D3DXMATRIX& mat)
	{
		XCALL(GothicMemoryLocations::zCCamera::SetTransform);
	}

	void Activate()
	{
		XCALL(GothicMemoryLocations::zCCamera::Activate);
	}

	void SetFOV(float azi, float elev)
	{
#ifndef BUILD_GOTHIC_1_08k // TODO: Implement this for G1
		XCALL(GothicMemoryLocations::zCCamera::SetFOV);
#endif
	}

	void UpdateViewport()
	{
		XCALL(GothicMemoryLocations::zCCamera::UpdateViewport);
	}

	zTCam_ClipType BBox3DInFrustum(const zTBBox3D& box)
	{
		//int flags = 15; // Full clip, no farplane
		int flags = 63;
		return BBox3DInFrustum(box, flags);
	}

	zTCam_ClipType BBox3DInFrustum(const zTBBox3D& box, int& clipFlags)
	{
		XCALL(GothicMemoryLocations::zCCamera::BBox3DInFrustum);
	}

	float GetFarPlane()
	{
#ifdef BUILD_GOTHIC_2_6_fix
		return *(float *)((char *)this + GothicMemoryLocations::zCCamera::Offset_FarPlane);
#else
		return 20000.0f;
#endif
	}

	float GetNearPlane()
	{
#ifdef BUILD_GOTHIC_2_6_fix
		return *(float *)((char *)this + GothicMemoryLocations::zCCamera::Offset_NearPlane);
#else
		return 0.5f;
#endif
	}

	void SetFarPlane(float value)
	{
#ifdef BUILD_GOTHIC_2_6_fix
		XCALL(GothicMemoryLocations::zCCamera::SetFarPlane);
#endif
	}

	/*void GetCameraPosition(D3DXVECTOR3& v)
	{
		XCALL(GADDR::zCCamera_GetCameraPosition);
	}*/

	/*static void SetFreeLook(bool freeLook)
	{
		bool* f = (bool *)GothicMemoryLocations::zCCamera::Var_FreeLook;
		*f = freeLook;
	}*/

	static zCCamera* GetCamera(){return *(zCCamera**)GothicMemoryLocations::GlobalObjects::zCCamera;}

	/** Frustum Planes in world space */
	zTPlane FrustumPlanes[6];

private:
};