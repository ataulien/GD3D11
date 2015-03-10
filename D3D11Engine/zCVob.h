#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zSTRING.h"
#include "zCObject.h"
#include "zCArray.h"

enum EVisualCamAlignType
{
	zVISUAL_CAM_ALIGN_NONE = 0,
	zVISUAL_CAM_ALIGN_YAW = 1,
	zVISUAL_CAM_ALIGN_FULL = 2
};

class zCVisual;
class zCBspLeaf;
class zCWorld;
class zCVob
{
public:
	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_zCVobSetVisual = (zCVobSetVisual)DetourFunction((BYTE *)GothicMemoryLocations::zCVob::SetVisual, (BYTE *)zCVob::Hooked_SetVisual);
	}

	static void __fastcall Hooked_SetVisual(void* thisptr, void* unknwn, zCVisual* visual)
	{
		hook_infunc

		HookedFunctions::OriginalFunctions.original_zCVobSetVisual(thisptr, visual);

		// Notify the world
		if(Engine::GAPI)
			Engine::GAPI->OnSetVisual((zCVob*)thisptr);

		hook_outfunc
	}

	zCVisual* GetVisual()
	{
		XCALL(GothicMemoryLocations::zCVob::GetVisual);
	}

	D3DXVECTOR3 GetPositionWorld() const
	{
		return D3DXVECTOR3(*(float *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_WorldPosX), 
			*(float *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_WorldPosY), 
			*(float *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_WorldPosZ));
		//XCALL(GothicMemoryLocations::zCVob::GetPositionWorld);
	}

	zTBBox3D GetBBoxLocal()
	{
		XCALL(GothicMemoryLocations::zCVob::GetBBoxLocal);
	}

	D3DXMATRIX* GetWorldMatrixPtr()
	{
		return (D3DXMATRIX *)(this + GothicMemoryLocations::zCVob::Offset_WorldMatrixPtr);
	}

	zCPolygon* GetGroundPoly()
	{
		return *(zCPolygon **)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_GroundPoly);
	}

	zCWorld* GetHomeWorld()
	{
		return *(zCWorld **)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_HomeWorld);
	}

	

	bool GetShowVisual()
	{
		unsigned int flags = *(unsigned int *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_Flags);

		return (flags & GothicMemoryLocations::zCVob::MASK_ShowVisual) != 0;
	}

	EVisualCamAlignType GetAlignment()
	{
		unsigned int flags = *(unsigned int *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_CameraAlignment);
		
		//.text:00601652                 shl     eax, 1Eh
		//.text:00601655                 sar     eax, 1Eh

		flags <<= GothicMemoryLocations::zCVob::SHIFTLR_CameraAlignment;
		flags >>= GothicMemoryLocations::zCVob::SHIFTLR_CameraAlignment;

		return (EVisualCamAlignType)flags;
	}

	

	/*void DoFrameActivity()
	{
		XCALL(GothicMemoryLocations::zCVob::DoFrameActivity);
	}*/

	void GetWorldMatrix(D3DXMATRIX* m)
	{
		*m = *GetWorldMatrixPtr();
	}

	/*zTBBox3D* GetBoundingBoxWS()
	{
		return (zTBBox3D *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_BoundingBoxWS); 
	}*/

	/** Data */
	/*zCTree<zCVob>* GlobalVobTreeNode;
	int LastTimeDrawn;
	DWORD LastTimeCollected;

	zCArray<zCBspLeaf*>	LeafList;
	D3DXMATRIX WorldMatrix;
	zTBBox3D BoundingBoxWS;*/
};
	