#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zSTRING.h"
#include "zCArray.h"
#include "zTypes.h"
#include "zCWorld.h"
#include "zCCamera.h"
#include "zCModelTexAniState.h"
#include "zCVisual.h"
#include "zCMeshSoftSkin.h"
#include "zCObject.h"

class zCVisual;
class zCMeshSoftSkin;

struct zCModelNode;
struct zCModelNodeInst
{
	zCModelNodeInst* ParentNode;
	zCModelNode* ProtoNode;
	zCVisual* NodeVisual;
	D3DXMATRIX Trafo;			
	D3DXMATRIX TrafoObjToCam;	
	zTBBox3D BBox3D;

	zCModelTexAniState TexAniState;
};



struct zCModelNode
{
	zCModelNode* ParentNode;
	zSTRING	NodeName;
	zCVisual* Visual;
	D3DXMATRIX Transform;

	D3DXVECTOR3 NodeRotAxis;
	float NodeRotAngle;
	D3DXVECTOR3	Translation;
	D3DXMATRIX TransformObjToWorld;
	D3DXMATRIX* NodeTransformList;
	zCModelNodeInst* LastInstNode;
};

struct zTMdl_NodeVobAttachment
{
	zCVob* Vob;
	zCModelNodeInst* NodeInst;
};

class zCModelMeshLib;
struct zTMeshLibEntry
{				
	zCModelTexAniState TexAniState;
	zCModelMeshLib* MeshLibPtr;
};

class zCModelAni
{
public:
	bool IsIdleAni()
	{
		DWORD value = *(DWORD *)THISPTR_OFFSET(GothicMemoryLocations::zCModelAni::Offset_Flags);
		return (value & GothicMemoryLocations::zCModelAni::Mask_FlagIdle) != 0;
	}

	int GetNumAniFrames()
	{
		return *(uint16_t *)THISPTR_OFFSET(GothicMemoryLocations::zCModelAni::Offset_NumFrames);
	}
};

class zCModelAniActive
{
public:
	zCModelAni* protoAni;
	zCModelAni* nextAni;
};

class zCModelMeshLib : public zCObject
{
public:
	struct zTNodeMesh
	{			
		zCVisual* Visual;
		int NodeIndex;	
	};

	/** This returns the list of nodes which hold information about the bones and attachments later */
	zCArray<zTNodeMesh>* GetNodeList()
	{
		return &NodeList;
	}

	/** Returns the list of meshes which store the vertex-positions and weights */
	zCArray<zCMeshSoftSkin *>* GetMeshSoftSkinList()
	{
#ifndef BUILD_GOTHIC_1_08k
		return &SoftSkinList;
#else
		return NULL;
#endif
	}

	const char* GetVisualName()
	{
		if(GetMeshSoftSkinList()->NumInArray > 0)
			return GetMeshSoftSkinList()->Array[0]->GetObjectName();

		return "";
		//return __GetVisualName().ToChar();
	}

private:
	zCArray<zTNodeMesh>			NodeList;
	zCArray<zCMeshSoftSkin*>	SoftSkinList;
};

class zCModelPrototype
{
public:
	/** Hooks the functions of this Class */
	static void Hook()
	{
		//HookedFunctions::OriginalFunctions.original_zCModelPrototypeLoadModelASC = (zCModelPrototypeLoadModelASC)DetourFunction((BYTE *)GothicMemoryLocations::zCModelPrototype::LoadModelASC, (BYTE *)zCModelPrototype::Hooked_LoadModelASC);
		//HookedFunctions::OriginalFunctions.original_zCModelPrototypeReadMeshAndTreeMSB = (zCModelPrototypeReadMeshAndTreeMSB)DetourFunction((BYTE *)GothicMemoryLocations::zCModelPrototype::ReadMeshAndTreeMSB, (BYTE *)zCModelPrototype::Hooked_ReadMeshAndTreeMSB);

	}

	/** This is called on load time for models */
	static int __fastcall Hooked_LoadModelASC(void* thisptr, void* unknwn, const zSTRING& file)
	{
		LogInfo() << "Loading Model: " << file.ToChar();
		int r = HookedFunctions::OriginalFunctions.original_zCModelPrototypeLoadModelASC(thisptr, file);

		// Pre-Load this model for us, too
		if(r)
		{
			
		}

		return r;
	}

	/** This is called on load time for models */
	static int __fastcall Hooked_ReadMeshAndTreeMSB(void* thisptr, void* unknwn, int& i, class zCFileBIN& f)
	{
		LogInfo() << "Loading Model!";
		int r = HookedFunctions::OriginalFunctions.original_zCModelPrototypeReadMeshAndTreeMSB(thisptr, i, f);

		// Pre-Load this model for us, too
		if(r)
		{
		}

		return r;
	}

	


	/** This returns the list of nodes which hold information about the bones and attachments later */
	zCArray<zCModelNode *>* GetNodeList()
	{
		return (zCArray<zCModelNode *> *)THISPTR_OFFSET(GothicMemoryLocations::zCModelPrototype::Offset_NodeList);
	}

	/** Returns the list of meshes which store the vertex-positions and weights */
	zCArray<zCMeshSoftSkin *>* GetMeshSoftSkinList()
	{
#ifndef BUILD_GOTHIC_1_08k
		return (zCArray<zCMeshSoftSkin *> *)THISPTR_OFFSET(GothicMemoryLocations::zCModelPrototype::Offset_MeshSoftSkinList);
#else
		return NULL;
#endif
	}

	/** Returns the name of the first Mesh inside this */
	const char* GetVisualName()
	{
		if(GetMeshSoftSkinList()->NumInArray > 0)
			return GetMeshSoftSkinList()->Array[0]->GetObjectName();

		return "";
	}
};



class zCModel : public zCVisual
{
public:
	/** Hooks the functions of this Class */
	static void Hook()
	{
/*#ifndef BUILD_GOTHIC_1_08k
		DWORD dwProtect;
		VirtualProtect((void *)GothicMemoryLocations::zCModel::AdvanceAnis, GothicMemoryLocations::zCModel::SIZE_AdvanceAnis, PAGE_EXECUTE_READWRITE, &dwProtect);

		byte unsmoothAnisFix[] = {0x75, 0x00, 0xC7, 0x44, 0x24, 0x78, 0x01, 0x00, 0x00, 0x00}; // Replaces a jnz in AdvanceAnis - Thanks to killer-m!
		memcpy((void *)GothicMemoryLocations::zCModel::RPL_AniQuality, unsmoothAnisFix, sizeof(unsmoothAnisFix));
#endif*/
	}


	/** Creates an array of matrices for the bone transforms */
	void __fastcall RenderNodeList( zTRenderContext& renderContext, zCArray<D3DXMATRIX*>& boneTransforms, zCRenderLightContainer& lightContainer, int lightingMode = 0)
	{
		XCALL(GothicMemoryLocations::zCModel::RenderNodeList);
	}

	/** Returns the current amount of active animations */
	int GetNumActiveAnimations()
	{
		return *(int *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_NumActiveAnis);
	}

	/** Returns true if only an idle-animation is running */
	bool IdleAnimationRunning()
	{
		zCModelAniActive* activeAni = *(zCModelAniActive **)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_AniChannels);

		return GetNumActiveAnimations() == 1 && (/*activeAni->protoAni->IsIdleAni() ||*/ activeAni->protoAni->GetNumAniFrames() <= 1);
	}

	/** This is needed for the animations to work at full framerate */
	void SetDistanceToCamera(float dist)
	{
		*(float *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_DistanceModelToCamera) = dist;
	}

	/** Updates stuff like blinking eyes, etc */
	void zCModel::UpdateMeshLibTexAniState() 
	{
		for (int i=0; i<GetMeshLibList()->NumInArray; i++) 
			GetMeshLibList()->Array[i]->TexAniState.UpdateTexList();
	}

	int GetIsVisible()
	{
#ifndef BUILD_GOTHIC_1_08k
		return (*(int *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_Flags)) & 1;
#else
		return 1;
#endif
	}

	void SetIsVisible(bool visible)
	{
#ifndef BUILD_GOTHIC_1_08k
		int v = visible ? 1 : 0;

		byte* flags = (byte *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_Flags);

		*flags &= ~1;
		*flags |= v;
#else
		// Do nothing yet
		// FIXME
#endif
	}

	D3DXVECTOR3 GetModelScale()
	{
#ifdef BUILD_GOTHIC_1_08k
		return D3DXVECTOR3(1,1,1);
#endif

		return *(D3DXVECTOR3 *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_ModelScale);
	}

	float GetModelFatness()
	{
#ifdef BUILD_GOTHIC_1_08k
		return 1.0f;
#endif
		return *(float *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_ModelFatness);
	}

	int GetDrawHandVisualsOnly()
	{
#ifndef BUILD_GOTHIC_1_08k
		return *(int *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_DrawHandVisualsOnly);
#else
		return 0; // First person not implemented in G1
#endif
	}

	zCArray<zCModelNodeInst *>* GetNodeList()
	{
		return (zCArray<zCModelNodeInst *> *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_NodeList);
	}

	zCArray<zCMeshSoftSkin *>* GetMeshSoftSkinList()
	{
		return (zCArray<zCMeshSoftSkin *> *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_MeshSoftSkinList);
	}

	zCArray<zCModelPrototype *>* GetModelProtoList()
	{
		return (zCArray<zCModelPrototype *> *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_ModelProtoList);
	}

	zCArray<zTMeshLibEntry *>* GetMeshLibList()
	{
		return (zCArray<zTMeshLibEntry *> *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_MeshLibList);
	}

	zCArray<zTMdl_NodeVobAttachment>* GetAttachedVobList()
	{
		return (zCArray<zTMdl_NodeVobAttachment> *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_AttachedVobList); 
	}

	/* Updates the world matrices of the attached VOBs */
	void UpdateAttachedVobs()
	{
		XCALL(GothicMemoryLocations::zCModel::UpdateAttachedVobs); 
	}

	/** Fills a vector of (viewspace) bone-transformation matrices for this frame */
	void GetBoneTransforms(std::vector<D3DXMATRIX>* transforms, zCVob* vob = NULL)
	{
		if(!GetNodeList())
			return;

		// Make this static so we don't reallocate the memory every time
		static std::vector<D3DXMATRIX*> tptr;
		tptr.resize(GetNodeList()->NumInArray, NULL);
		for (int i=0; i<GetNodeList()->NumInArray; i++) 
		{
			zCModelNodeInst* node = GetNodeList()->Array[i];
			zCModelNodeInst* parent = node->ParentNode;
			tptr[i] = &node->TrafoObjToCam;

			// Calculate transform for this node
			if(parent)	
			{
				node->TrafoObjToCam = parent->TrafoObjToCam * node->Trafo;
			}else
			{
				node->TrafoObjToCam = node->Trafo;
			}
		
		}
		// Put them into our vector
		for(unsigned int i=0;i<tptr.size();i++)
		{
			D3DXMATRIX m = *tptr[i];
			transforms->push_back(m);
		}
	}

	const char* GetVisualName()
	{
		if(GetMeshSoftSkinList()->NumInArray > 0)
			return GetMeshSoftSkinList()->Array[0]->GetObjectName();

		return "";
		//return __GetVisualName().ToChar();
	}

	zSTRING GetModelName()
	{
		XCALL(GothicMemoryLocations::zCModel::GetVisualName);
	}

private:
	
};