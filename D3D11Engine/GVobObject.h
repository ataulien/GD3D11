#pragma once
#include "pch.h"
#include "ConstantBufferStructs.h"

/** Wrappper around the zCVob-Object */

class zCVob;
class GVisual;
class BaseConstantBuffer;
struct BspNodeInfo;
struct PipelineState;
class GVobObject
{
public:
	GVobObject(zCVob* sourceVob, GVisual* visual);
	virtual ~GVobObject(void);

	/** Updates this vobs constantbuffer */
	void UpdateVobConstantBuffer();

	/** Draws this vob using its visual */
	void DrawVob();

	/** Adds a BSP-Node to the list of the nodes containing this vob */
	void AddParentBSPNode(BspNodeInfo* node);

	/** Returns the list of BSP-Nodes containing this vob */
	const std::vector<BspNodeInfo*>& GetParentBSPNodes();

	/** Returns whether this vob has already been collected in the last tree-search */
	bool IsAlreadyCollectedFromTree();

	/** Sets that the vob has been collected in the last tree-search */
	void SetCollectedFromTreeSearch();

	/** Resets the "already found"-flag */
	void ResetTreeSearchState();

	/** Returns the position of this vob */
	const D3DXVECTOR3& GetPosition();

	/** Returns the source vob of this object */
	zCVob* GetSourceVob();

	/** Returns the visual of this object */
	GVisual* GetVisual();

	/** Sets a general-purpose value for the vob-constantbuffer */
	void SetVobInstanceGPSlot(UINT slot, DWORD value);

	/** Returns the current instance data */
	const VobInstanceInfo& GetInstanceInfo();

	/** Sets the instance-index */
	void SetInstanceIndex(unsigned short idx);

	/** Returns the current instance-index */
	const VobInstanceRemapInfo& GetInstanceRemapInfo();

	/** Returns whether this vob is dynamic or not */
	bool IsDynamic();

	/** Sets whether this vob is dynamic or not */
	void SetDynamic(bool value);

	/** Returns the vector of pipelinestates */
	std::vector<PipelineState*>& GetPipelineStates();
private:

	/** Source Vob-Object */
	zCVob* SourceVob;
	GVisual* Visual;

	/** Constantbuffer for this vob */
	BaseConstantBuffer* VobConstantBuffer;

	/** Pipelinestates to render this vob with. This needs to be here
		since every vob can have different attributes for a specific visual. */
	std::vector<PipelineState*> RenderPipelineStates;

	/** Current instance info for this vob 
		* Contains worldmatrix and ground color for example.
		* Updated with UpdateVobConstantBuffer */
	VobInstanceInfo InstanceInfo;

	/** BSP-Node this is stored in */
	std::vector<BspNodeInfo*> ParentBSPNodes;

	/** Vob state */
	struct {
		/** Whether this vob is currently in an indoor location or not */
		bool IsIndoorVob;

		/** Light-Color of the ground under this vob */
		DWORD GroundLightColor;

		/** Whether this vob was already collected in the last tree-search */
		bool AlreadyFoundInLastTreeSearch;

		/** If true, this vob is seen as a moving vob and is save to move out if it's bsp-node */
		bool IsDynamicVob;

		/** Instance index for this vob */
		VobInstanceRemapInfo InstanceRemapInfo;
	} State;
};

