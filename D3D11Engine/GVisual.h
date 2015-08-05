#pragma once
#include "zCVob.h"

/** If defined, the default visuals will draw themselfes using lines */
//#define DEBUG_DRAW_VISUALS

class zCVisual;
class GVobObject;
struct PipelineState;
class BaseConstantBuffer;
struct RenderInfo
{
	RenderInfo(GVobObject* vob, D3DXMATRIX* mat, BaseConstantBuffer* instanceCB = NULL, float distance = 0.0f)
	{
		CallingVob = vob;
		WorldMatrix = mat;
		InstanceCB = instanceCB;
		Distance = distance;
	}

	GVobObject* CallingVob;
	D3DXMATRIX* WorldMatrix;
	BaseConstantBuffer* InstanceCB;
	float Distance;
};

/** Generic visual wrapper */
class GVisual
{
public:
	GVisual(zCVisual* sourceVisual);
	virtual ~GVisual(void);

	/** Called on a new frame */
	virtual void OnBeginDraw(){}

	/** Called when we are done drawing */
	virtual void OnEndDraw();

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);

	/** Fills the given pipeline state with it the visuals current settings */
	virtual void FillPipelineState(PipelineState* state){}

	/** Just adds a static instance */
	virtual int* AddStaticInstance(const VobInstanceRemapInfo& remapInfo);

	/** Draws a batch of instance-infos. Returns a pointer to the instance-buffer and it's size.
		If the buffer is too small use .*/
	virtual void BeginDrawInstanced(){}

	/** Can be called before you add instances to the buffer, so the visual can increase the size of the instancing buffer if needed 
		Returns false if the buffer wasn't big enough and had to be recreated.*/
	virtual bool OnAddInstances(int numInstances, VobInstanceInfo* instances){return true;}


	/** Finishes the instanced-draw-call */
	virtual void EndDrawInstanced(){}

	/** Switches the resources so we can have multiple states on this visual.
		The BSP-Tree needs to grab the instancing-buffers for this for example,
		and every node needs its own version */
	virtual void SwitchInstanceSpecificResources(){}

	/** Returns the size of this visual */
	float GetVisualSize();

	/** Returns the visual this is based on */
	zCVisual* GetSourceVisual();

	/** If this returns true, that vob will be cached in the bsp-tree using instancing */
	virtual bool IsInstancingCapable(){return false;}

	/** Returns the current instance-data, if there is any */
	virtual std::vector<VobInstanceInfo>* GetInstanceData(){return NULL;}
protected:
	/** Original visual from the game */
	zCVisual* SourceVisual;

	/** Size of this visual */
	float VisualSize;

	/** Whether this has been drawn this frame or not */
	bool DrawnThisFrame;
};

