#include "pch.h"
#include "GVobObject.h"
#include "ConstantBufferStructs.h"
#include "zCVob.h"
#include "GVisual.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"
#include "Toolbox.h"

GVobObject::GVobObject(zCVob* sourceVob, GVisual* visual)
{
	SourceVob = sourceVob;
	Visual = visual;

	State.IsIndoorVob = sourceVob->IsIndoorVob();
	State.GroundLightColor = 0xFFFFFFFF;
	State.IsIndoorVob = false;
	State.AlreadyFoundInLastTreeSearch = false;
	State.IsDynamicVob = false;

	Engine::GraphicsEngine->CreateConstantBuffer(&VobConstantBuffer, NULL, sizeof(VS_ExConstantBuffer_PerInstance));
	UpdateVobConstantBuffer();
}


GVobObject::~GVobObject(void)
{
	Toolbox::DeleteElements(RenderPipelineStates);
}

/** Returns whether this vob is dynamic or not */
bool GVobObject::IsDynamic()
{
	return State.IsDynamicVob;
}


/** Sets whether this vob is dynamic or not */
void GVobObject::SetDynamic(bool value)
{
	State.IsDynamicVob = value;
}

/** Returns the vector of pipelinestates */
std::vector<PipelineState*>& GVobObject::GetPipelineStates()
{
	return RenderPipelineStates;
}

/** Draws this vob using its visual */
void GVobObject::DrawVob()
{
	//Engine::GraphicsEngine->GetLineRenderer()->AddPointLocator(GetPosition());

	// Draw the visual registered here
	// TODO: Distance would already calculated in the BSP-Tree!
	Visual->DrawVisual(RenderInfo(this, SourceVob->GetWorldMatrixPtr(), VobConstantBuffer, 0.0f));
}

/** Updates this vobs constantbuffer */
void GVobObject::UpdateVobConstantBuffer()
{
	VS_ExConstantBuffer_PerInstance cb;
	cb.World = *SourceVob->GetWorldMatrixPtr();
	VobConstantBuffer->UpdateBuffer(&cb);

	// Colorize the vob according to the underlaying polygon
	if(State.IsIndoorVob)
	{
		// All lightmapped polys have this color, so just use it
		State.GroundLightColor = DEFAULT_LIGHTMAP_POLY_COLOR;
	}else
	{
		// Get the color of the first found feature of the ground poly
		State.GroundLightColor = SourceVob->GetGroundPoly() ? SourceVob->GetGroundPoly()->getFeatures()[0]->lightStatic : 0xFFFFFFFF;
	}

	InstanceInfo.world = cb.World;
	InstanceInfo.color = State.GroundLightColor;
}

/** Returns the source vob of this object */
zCVob* GVobObject::GetSourceVob()
{
	return SourceVob;
}

/** Returns the current instance data */
const VobInstanceInfo& GVobObject::GetInstanceInfo()
{
	return InstanceInfo;
}

/** Adds a BSP-Node to the list of the nodes containing this vob */
void GVobObject::AddParentBSPNode(BspNodeInfo* node)
{
	ParentBSPNodes.push_back(node);
}

/** Returns the list of BSP-Nodes containing this vob */
const std::vector<BspNodeInfo*>& GVobObject::GetParentBSPNodes()
{
	return ParentBSPNodes;
}

/** Returns whether this vob has already been collected in the last tree-search */
bool GVobObject::IsAlreadyCollectedFromTree()
{
	return State.AlreadyFoundInLastTreeSearch;
}

/** Sets that the vob has been collected in the last tree-search */
void GVobObject::SetCollectedFromTreeSearch()
{
	State.AlreadyFoundInLastTreeSearch = true;
}

/** Resets the "already found"-flag */
void GVobObject::ResetTreeSearchState()
{
	State.AlreadyFoundInLastTreeSearch = false;
}

/** Returns the visual of this object */
GVisual* GVobObject::GetVisual()
{
	return Visual;
}

/** Returns the position of this vob */
const D3DXVECTOR3& GVobObject::GetPosition()
{
	return SourceVob->GetPositionWorld();
}