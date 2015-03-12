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

struct zTMeshLibEntry
{				
	zCModelTexAniState	TexAniState;
	int MeshLibPtr;
};

class zCModelPrototype
{
public:
	zCArray<zCModelNode *>* GetNodeList()
	{
		return (zCArray<zCModelNode *> *)THISPTR_OFFSET(GothicMemoryLocations::zCModelPrototype::Offset_NodeList);
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

	void SetDistanceToCamera(float dist)
	{
		*(float *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_DistanceModelToCamera) = dist;
	}

	void zCModel::UpdateMeshLibTexAniState() 
	{
		for (int i=0; i<GetMeshLibList()->NumInArray; i++) 
			GetMeshLibList()->Array[i]->TexAniState.UpdateTexList();
	}

	int GetIsVisible()
	{
		return (*(int *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_Flags)) & 1;
	}

	void SetIsVisible(bool visible)
	{
		int v = visible ? 1 : 0;

		byte* flags = (byte *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_Flags);

		*flags &= ~1;
		*flags |= v;
	}

	D3DXVECTOR3 GetModelScale()
	{
		return *(D3DXVECTOR3 *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_ModelScale);
	}

	float GetModelFatness()
	{
		return *(float *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_ModelFatness);
	}

	int GetDrawHandVisualsOnly()
	{
		return *(int *)THISPTR_OFFSET(GothicMemoryLocations::zCModel::Offset_DrawHandVisualsOnly);
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
	void GetBoneTransforms(std::vector<D3DXMATRIX>* transforms, zCVob* vob)
	{
		if(!GetNodeList())
			return;

		std::vector<D3DXMATRIX*> tptr;
		tptr.resize(GetNodeList()->NumInArray);
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
			transforms->push_back((*tptr[i]));
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