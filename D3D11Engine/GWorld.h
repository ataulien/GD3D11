#pragma once
#include "pch.h"
#include "WorldConverter.h"
#include "BaseGraphicsEngine.h"

/** Currently loaded world */
class zCPolygon;
struct MeshInfo;
class GVisual;
class zCBspBase;
class GVobObject;
struct BspNodeInfo
{
	BspNodeInfo()
	{
		OriginalNode = NULL;
		Front = NULL;
		Back = NULL;
		NumLevels = 0;
		OcclusionInfo.VisibleLastFrame = false;
		OcclusionInfo.LastVisitedFrameID = 0;
		OcclusionInfo.QueryID = -1;
		OcclusionInfo.QueryInProgress = false;
		OcclusionInfo.LastCameraClipType = 0;

		OcclusionInfo.NodeMesh = NULL;
	}

	~BspNodeInfo()
	{
		delete OcclusionInfo.NodeMesh;

		for(auto it = StaticObjectPipelineStates.begin();it != StaticObjectPipelineStates.end();it++)
		{
			// These are all only valid for this particular node
			// so we need to delete them here
			delete (*it)->AssociatedState->BaseState.VertexBuffers[1]; // This should hold an instancingbuffer, which is only valid for this node if its in here
			delete (*it);
		}
	}

	bool IsEmpty()
	{
		return Vobs.empty() && IndoorVobs.empty() && SmallVobs.empty() && Lights.empty() && IndoorLights.empty();
	}

	void RemoveVob(GVobObject* vob)
	{
		// Erase from all lists, expensive!
		Toolbox::EraseByElement(Vobs, vob);
		Toolbox::EraseByElement(IndoorVobs, vob);
		Toolbox::EraseByElement(SmallVobs, vob);
		Toolbox::EraseByElement(Lights, vob);
		Toolbox::EraseByElement(IndoorLights, vob);
	}

	/** Calculates the level of this node and all subnodes */
	int CalculateLevels();

	/** Vob-lists for this node */
	std::vector<GVobObject *> Vobs;
	std::vector<GVobObject *> IndoorVobs;
	std::vector<GVobObject *> SmallVobs;
	std::vector<GVobObject *> Lights;
	std::vector<GVobObject *> IndoorLights;

	// This is filled in case we have loaded a custom worldmesh
	std::vector<zCPolygon *> NodePolygons;

	/** Pipeline-states for everything static in this Node */
	std::vector<PipelineState::PipelineSortItem*> StaticObjectPipelineStates;

	/** Cache for instancing-info, so we can copy it to the buffers in batches */
	std::map<GVisual*, std::vector<VobInstanceInfo>> InstanceDataCache;

	/** Num of sub-nodes this node has */
	unsigned int NumLevels;

	/** Occlusion info for this node */
	struct OcclusionInfo_s
	{
		unsigned int LastVisitedFrameID;
		bool VisibleLastFrame;
		int QueryID;
		bool QueryInProgress;
		MeshInfo* NodeMesh;
		int LastCameraClipType;
	} OcclusionInfo;

	// Original bsp-node
	zCBspBase* OriginalNode;
	BspNodeInfo* Front;
	BspNodeInfo* Back;
};

class GVobObject;
class GWorldMesh;
class GSkeletalMeshVisual;
class zCModelPrototype;

struct BspInfo;
class zCVob;
class GWorld
{
public:
	GWorld(void);
	virtual ~GWorld(void);

	/** Called on render */
	virtual void DrawWorld();

	/** (Re)loads the world mesh */
	void ExtractWorldMesh(zCBspTree* bspTree);

	/** Extracts all vobs from the bsp-tree, resets old tree */
	void BuildBSPTree(zCBspTree* bspTree);

	/** Called when the game wanted to add a vob to the world */
	void AddVob(zCVob* vob, zCWorld* world, bool forceDynamic = false);

	/** Removes a vob from the world, returns false if it wasn't registered */
	bool RemoveVob(zCVob* vob, zCWorld* world);

	/** Creates the right type of visual from the source */
	GVisual* CreateVisualFrom(zCVisual* sourceVisual);

	/** Returns the matching GVisual from the given zCVisual */
	GVisual* GetVisualFrom(zCVisual* sourceVisual);

	/** Creates a model-proto form the given object */
	GSkeletalMeshVisual* CreateModelProtoFrom(zCModelMeshLib* sourceProto);

	/** Returns the matching GSkeletalMeshVisual from the given zCModelPrototype */
	GSkeletalMeshVisual* GetModelProtoFrom(zCModelMeshLib* sourceProto);
protected:
	/** Begins the frame on visuals */
	void BeginVisualFrame();

	/** Ends the frame on visuals */
	void EndVisualFrame();

	/** Registers a single vob in all needed data structures */
	void RegisterSingleVob(GVobObject* vob, bool forceDynamic = false);

	/** Unregisters a single vob from all data structures */
	void UnregisterSingleVob(GVobObject* vob);

	/** Helper function for going through the bsp-tree to collect all the vobs*/
	void BuildBspTreeHelper(zCBspBase* base);

	/** Draws all visible vobs in the tree */
	void DrawBspTreeVobs(BspNodeInfo* base);

	/** Draws all vobs in the given node */
	void DrawBspNodeVobs(BspNodeInfo* node, float nodeDistance = 0.0f);

	/** Draws all vobs in the given node */
	void PrepareBspNodeVobPipelineStates(BspNodeInfo* node, std::set<GVisual*>& drawnVisuals, std::vector<GVobObject*>& drawnVobs);

	/** Draws all visible vobs in the tree */
	void PrepareBspTreePipelineStates(BspNodeInfo* base, std::set<GVisual*>& drawnVisuals, std::vector<GVobObject*>& drawnVobs);

	/** Draws the prepared pipelinestates of a BSP-Node */
	void DrawPreparedPipelineStates(BspNodeInfo* node);
	void DrawPreparedPipelineStatesRec(BspNodeInfo* base, zTBBox3D boxCell, int clipFlags);

	/** Prepares all visible vobs in the tree */
	void PrepareBspTreeVobs(BspNodeInfo* base);

	/** Resets all vobs in the BspDrawnVobs-Vector */
	void ResetBspDrawnVobs();

	/** Map of original vobs to our counterparts */
	std::hash_map<zCVob*, GVobObject*> VobMap;

	/** List of static vobs. These are also registered in the BSP-Tree. */
	std::vector<GVobObject*> StaticVobList;

	/** List of dynamic vobs, these are not registered in the BSP-Tree */
	std::vector<GVobObject*> DynamicVobList;

	/** The currently loaded world mesh */
	GWorldMesh* WorldMesh;

	/** Map of VobInfo-Lists for zCBspLeafs */
	std::hash_map<zCBspBase *, BspNodeInfo*> BspMap;

	/** Map of Visuals and our counterparts */
	std::hash_map<zCVisual *, GVisual *> VisualMap;

	/** Map of Model-Prototypes and our counterparts */
	std::hash_map<zCModelMeshLib*, GSkeletalMeshVisual*> ModelProtoMap;

	/** List of vobs drawn from the last BuildBspTree-Call */
	std::vector<GVobObject *> BspDrawnVobs;
};

