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
		HookedFunctions::OriginalFunctions.original_zCVobDestructor = (GenericDestructor)DetourFunction((BYTE *)GothicMemoryLocations::zCVob::Destructor, (BYTE *)zCVob::Hooked_Destructor);

		HookedFunctions::OriginalFunctions.original_zCVobEndMovement = (zCVobEndMovement)DetourFunction((BYTE *)GothicMemoryLocations::zCVob::EndMovement, (BYTE *)zCVob::Hooked_EndMovement);
	}

	/** Called when this vob got it's world-matrix changed */
#ifdef BUILD_GOTHIC_1_08k
	static void __fastcall Hooked_EndMovement(void* thisptr, void* unknwn)
	{
		hook_infunc

		HookedFunctions::OriginalFunctions.original_zCVobEndMovement(thisptr);

		if(Engine::GAPI)
			Engine::GAPI->OnVobMoved((zCVob *)thisptr);

		hook_outfunc
	}
#else
	static void __fastcall Hooked_EndMovement(void* thisptr, void* unknwn, int transformChanged) // G2 has one parameter more
	{
		hook_infunc

		HookedFunctions::OriginalFunctions.original_zCVobEndMovement(thisptr, transformChanged);

		if(Engine::GAPI && transformChanged)
			Engine::GAPI->OnVobMoved((zCVob *)thisptr);

		hook_outfunc
	}
#endif

	/** Called on destruction */
	static void __fastcall Hooked_Destructor(void* thisptr, void* unknwn)
	{
		hook_infunc

		// Notify the world. We are doing this here for safety so nothing possibly deleted remains in our world.
		if(Engine::GAPI)
			Engine::GAPI->OnRemovedVob((zCVob *)thisptr, ((zCVob *)thisptr)->GetHomeWorld());

		HookedFunctions::OriginalFunctions.original_zCVobDestructor(thisptr);

		hook_outfunc
	}

	/** Called when this vob is about to change the visual */
	static void __fastcall Hooked_SetVisual(void* thisptr, void* unknwn, zCVisual* visual)
	{
		hook_infunc

		HookedFunctions::OriginalFunctions.original_zCVobSetVisual(thisptr, visual);

		// Notify the world
		if(Engine::GAPI)
			Engine::GAPI->OnSetVisual((zCVob*)thisptr);

		hook_outfunc
	}

#ifdef BUILD_SPACER
	/** Returns the helper-visual for this class
		This actually uses a map to lookup the visual. Beware for performance-issues! */
	zCVisual* GetClassHelperVisual()
	{
		XCALL(GothicMemoryLocations::zCVob::GetClassHelperVisual);
	}

	/** Returns the visual saved in this vob */
	zCVisual* GetVisual()
	{
		zCVisual* visual = GetMainVisual();

		if(!visual)
			visual = GetClassHelperVisual();

		return visual;
	}
#else
	/** Returns the visual saved in this vob */
	zCVisual* GetVisual()
	{
		return GetMainVisual();
	}
#endif

#ifdef BUILD_GOTHIC_1_08k
	void _EndMovement()
	{
		XCALL(GothicMemoryLocations::zCVob::EndMovement);
	}
#else
	void _EndMovement(int p=1)
	{
		XCALL(GothicMemoryLocations::zCVob::EndMovement);
	}
#endif

	/** Updates the vobs transforms */
	void EndMovement()
	{
		_EndMovement();
	}

	/** Returns the visual saved in this vob */
	zCVisual* GetMainVisual()
	{
		XCALL(GothicMemoryLocations::zCVob::GetVisual);
	}

	/** Returns the name of this vob */
	std::string GetName()
	{
		return __GetObjectName().ToChar();
	}

	/** Returns the world-position of this vob */
	D3DXVECTOR3 GetPositionWorld() const
	{
		// Get the data right off the memory to save a function call
		return D3DXVECTOR3(*(float *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_WorldPosX), 
			*(float *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_WorldPosY), 
			*(float *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_WorldPosZ));
		//XCALL(GothicMemoryLocations::zCVob::GetPositionWorld);
	}

	/** Sets this vobs position */
	void SetPositionWorld(const D3DXVECTOR3& v) 
	{
#ifdef BUILD_SPACER
		XCALL(GothicMemoryLocations::zCVob::SetPositionWorld);
#endif
	}

	/** Returns the local bounding box */
	zTBBox3D GetBBoxLocal()
	{
		XCALL(GothicMemoryLocations::zCVob::GetBBoxLocal);
	}

	/** Returns a pointer to this vobs world-matrix */
	D3DXMATRIX* GetWorldMatrixPtr()
	{
		return (D3DXMATRIX *)(this + GothicMemoryLocations::zCVob::Offset_WorldMatrixPtr);
	}

	/** Copys the world matrix into the given memory location */	
	void GetWorldMatrix(D3DXMATRIX* m)
	{
		*m = *GetWorldMatrixPtr();
	}

	/** Returns the world-polygon right under this vob */
	zCPolygon* GetGroundPoly()
	{
		return *(zCPolygon **)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_GroundPoly);
	}

	/** Returns whether this vob is currently in an indoor-location or not */
	bool IsIndoorVob()
	{
		if(!GetGroundPoly())
			return false;

		return GetGroundPoly()->GetLightmap() != NULL;
	}

	/** Returns the world this vob resists in */
	zCWorld* GetHomeWorld()
	{
		return *(zCWorld **)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_HomeWorld);
	}

	/** Returns whether this vob is currently in sleeping state or not. Sleeping state is something like a waiting (cached out) NPC */
	int GetSleepingMode()
	{
		unsigned int flags = *(unsigned int *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_SleepingMode);

		return (flags & GothicMemoryLocations::zCVob::MASK_SkeepingMode);
	}

	/** Returns whether the visual of this vob is visible */
	bool GetShowVisual()
	{
		unsigned int flags = *(unsigned int *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_Flags);

#ifndef BUILD_SPACER
		return GetShowMainVisual();
#else
		// Show helpers in spacer if wanted
		bool showHelpers = (*(int *)GothicMemoryLocations::zCVob::s_ShowHelperVisuals) != 0;
		return GetShowMainVisual() || showHelpers;
#endif
	}

	/** Returns whether to show the main visual or not. Only used for the spacer */
	bool GetShowMainVisual()
	{
		unsigned int flags = *(unsigned int *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_Flags);

		return (flags & GothicMemoryLocations::zCVob::MASK_ShowVisual) != 0;
	}

	/** Alignemt to the camera */
	EVisualCamAlignType GetAlignment()
	{
		unsigned int flags = *(unsigned int *)THISPTR_OFFSET(GothicMemoryLocations::zCVob::Offset_CameraAlignment);
		
		//.text:00601652                 shl     eax, 1Eh
		//.text:00601655                 sar     eax, 1Eh

		flags <<= GothicMemoryLocations::zCVob::SHIFTLR_CameraAlignment;
		flags >>= GothicMemoryLocations::zCVob::SHIFTLR_CameraAlignment;

		return (EVisualCamAlignType)flags;
	}

protected:
	zSTRING& __GetObjectName()
	{
		XCALL(GothicMemoryLocations::zCObject::GetObjectName);
	}
	

	/*void DoFrameActivity()
	{
		XCALL(GothicMemoryLocations::zCVob::DoFrameActivity);
	}*/



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
	