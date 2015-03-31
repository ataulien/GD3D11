#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCVob.h"
#include "zCVisual.h"
#include "zCArray.h"
#include "zCMesh.h"
#include "zTypes.h"
#include "Logger.h"
#include "BaseGraphicsEngine.h"

class zCFileBIN;
class zCVob;
class zCBspLeaf;
class zCBspNode;
class zCBspBase;

enum zTBspNodeType 
{ 
	zBSP_LEAF=1, 
	zBSP_NODE=0 
};

enum zTBspMode
{
	zBSP_MODE_INDOOR = 0,
	zBSP_MODE_OUTDOOR = 1
};

class zCBspBase
{
public:
	zCBspNode* Parent;
	zTBBox3D BBox3D;							
	zCPolygon** PolyList;
	int	NumPolys;						
	zTBspNodeType NodeType;
				
	bool IsLeaf()
	{
		return NodeType == zBSP_LEAF;
	}

	bool IsNode()
	{
		return NodeType == zBSP_NODE;
	}
};

class zCBspNode : public zCBspBase
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		
		DetourFunction((BYTE *)GothicMemoryLocations::zCBspTree::Render, (BYTE *)zCBspNode::hooked_zCBspNodeRender);

		//(zCBspNodeRenderIndoor)DetourFunction((BYTE *)GothicMemoryLocations::zCBspNode::RenderIndoor, (BYTE *)zCBspNode::hooked_zCBspNodeRenderIndoor);
		//(zCBspNodeRenderOutdoor)DetourFunction((BYTE *)GothicMemoryLocations::zCBspNode::RenderOutdoor, (BYTE *)zCBspNode::hooked_zCBspNodeRenderOutdoor);
	
		/*DWORD dwProtect;
		VirtualProtect((void *)GothicMemoryLocations::zCBspNode::RenderOutdoor, GothicMemoryLocations::zCBspNode::SIZE_RenderOutdoor, PAGE_EXECUTE_READWRITE, &dwProtect);
		VirtualProtect((void *)GothicMemoryLocations::zCBspNode::RenderIndoor, GothicMemoryLocations::zCBspNode::REPL_RenderIndoorEnd - GothicMemoryLocations::zCBspNode::RenderIndoor, PAGE_EXECUTE_READWRITE, &dwProtect);
		
		
		// NOP some render-calls
		REPLACE_RANGE(GothicMemoryLocations::zCBspNode::RenderOutdoor, GothicMemoryLocations::zCBspNode::REPL_RenderOutdoorEnd - 1, INST_NOP);
		REPLACE_RANGE(GothicMemoryLocations::zCBspNode::RenderIndoor, GothicMemoryLocations::zCBspNode::REPL_RenderIndoorEnd - 1, INST_NOP);
		*/
		/*REPLACE_CALL(GothicMemoryLocations::zCBspTree::CALL_RenderOutdoor, INST_NOP);
		REPLACE_CALL(GothicMemoryLocations::zCBspTree::CALL_RenderOutdoor2, INST_NOP);
		REPLACE_CALL(GothicMemoryLocations::zCBspTree::CALL_RenderIndoor, INST_NOP);
		REPLACE_CALL(GothicMemoryLocations::zCBspTree::CALL_RenderIndoor2, INST_NOP);
		REPLACE_CALL(GothicMemoryLocations::zCBspTree::CALL_RenderIndoor3, INST_NOP);
		REPLACE_CALL(GothicMemoryLocations::zCBspTree::CALL_RenderIndoor4, INST_NOP);
		REPLACE_CALL(GothicMemoryLocations::zCBspTree::CALL_RenderTrivIndoor, INST_NOP);*/
	}
	
	static void __fastcall hooked_zCBspNodeRender(void* thisptr, void* unkwn)
	{
		// Start world rendering here
		Engine::GraphicsEngine->OnStartWorldRendering();
	}

	static void __fastcall hooked_zCBspNodeRenderIndoor(void* thisptr, int clipFlags)
	{
		LogInfo() << "Render indoor!";
		//HookedFunctions::OriginalFunctions.original_zCBspTreeAddVob(thisptr, vob);
	}

	static void __fastcall hooked_zCBspNodeRenderOutdoor(void* thisptr, zCBspBase* node, zTBBox3D bbox, int clipFlags, int crossingVobPlane)
	{
		LogInfo() << "Render outdoor!";
		//HookedFunctions::OriginalFunctions.original_zCBspTreeAddVob(thisptr, vob);
	}

	zTPlane	Plane;
	zCBspBase* Front;			
	zCBspBase* Back;		
	zCBspLeaf* LeafList;
	int NumLeafs;
	char PlaneSignbits;
};

class zCVobLight;
class zCBspLeaf : public zCBspBase
{
public:
	int LastTimeLighted;
	zCArray<zCVob*>	LeafVobList;
	zCArray<zCVobLight*> LightVobList;
};

/** BspTree-Object which holds the world */
class zCBspTree
{
public:
	/** Hooks the functions of this Class */
	static void Hook()
	{
		HookedFunctions::OriginalFunctions.original_zCBspTreeLoadBIN = (zCBspTreeLoadBIN)DetourFunction((BYTE *)GothicMemoryLocations::zCBspTree::LoadBIN, (BYTE *)zCBspTree::hooked_LoadBIN);
		HookedFunctions::OriginalFunctions.original_zCBspTreeAddVob = (zCBspTreeAddVob)DetourFunction((BYTE *)GothicMemoryLocations::zCBspTree::AddVob, (BYTE *)zCBspTree::hooked_AddVob);
	}

	/** Called when a vob gets added to a bsp-tree */
	static void __fastcall hooked_AddVob(void* thisptr, void* unknwn, zCVob* vob)
	{
		HookedFunctions::OriginalFunctions.original_zCBspTreeAddVob(thisptr, vob);

		if(vob->GetVisual())
		{
			//LogInfo() << vob->GetVisual()->GetFileExtension(0);
			//Engine::GAPI->OnAddVob(vob);
		}
	}

	/** Called on level load. */
	static int __fastcall hooked_LoadBIN(void* thisptr, void* unknwn, zCFileBIN& file, int skip)
	{
		LogInfo() << "Loading world!";
		int r = HookedFunctions::OriginalFunctions.original_zCBspTreeLoadBIN(thisptr, file, skip);

		zCBspTree* tree = (zCBspTree *)thisptr;
		LogInfo() << "World loaded, getting Levelmesh now!";
		LogInfo() << " - Found " << tree->GetNumPolys() << " polygons";

//#ifdef BUILD_GOTHIC_1_08k
		std::vector<zCPolygon *> polys;
		tree->GetLOD0Polygons(polys);
		
		Engine::GAPI->OnGeometryLoaded(&polys[0], polys.size());
/*#else
		Engine::GAPI->OnGeometryLoaded(tree->GetPolygons());
#endif*/

		// Save pointer to this
		Engine::GAPI->GetLoadedWorldInfo()->BspTree = tree;

		return r;
	}

	/** Returns only the polygons used in LOD0 of the world */
	void GetLOD0Polygons(std::vector<zCPolygon *>& target)
	{
		int num = GetNumLeafes();

		for(int i=0;i<num;i++)
		{
			zCBspLeaf* leaf = GetLeaf(i);

			for(int i=0;i<leaf->NumPolys;i++)
			{
				target.push_back(leaf->PolyList[i]);
			}
		}

		/*if(nodeBase->IsLeaf())
		{
			zCBspLeaf* leaf = (zCBspLeaf *)nodeBase;

			for(int i=0;i<leaf->NumPolys;i++)
			{
				target.push_back(leaf->PolyList[i]);
			}
		}else
		{
			zCBspNode* node = (zCBspNode *)nodeBase;
			GetLOD0Polygons(node->Front, target);
			GetLOD0Polygons(node->Back, target);
		}*/
	}

	int GetNumLeafes()
	{
		return *(int*)THISPTR_OFFSET(GothicMemoryLocations::zCBspTree::Offset_NumLeafes);
	}

	zTBspMode GetBspTreeMode()
	{
		return *(zTBspMode*)THISPTR_OFFSET(GothicMemoryLocations::zCBspTree::Offset_BspTreeMode);
	}

	

	zCBspLeaf* GetLeaf(int i)
	{
		char* list = *(char **)THISPTR_OFFSET(GothicMemoryLocations::zCBspTree::Offset_LeafList);
		return (zCBspLeaf*)(list + GothicMemoryLocations::zCBspLeaf::Size * i);
	}

	zCBspBase* GetRootNode()
	{
		return *(zCBspBase **)THISPTR_OFFSET(GothicMemoryLocations::zCBspTree::Offset_RootNode);
	}

	int GetNumPolys()
	{
		return *(int *)THISPTR_OFFSET(GothicMemoryLocations::zCBspTree::Offset_NumPolys);
	}

	zCPolygon** GetPolygons()
	{
		return *(zCPolygon ***)THISPTR_OFFSET(GothicMemoryLocations::zCBspTree::Offset_PolyArray);
	}

	zCMesh* GetMesh()
	{
		return *(zCMesh **)THISPTR_OFFSET(GothicMemoryLocations::zCBspTree::Offset_WorldMesh);
	}

private:

};

