#pragma once

/** If defined, the default visuals will draw themselfes using lines */
//#define DEBUG_DRAW_VISUALS

class zCVisual;
class GVobObject;

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
	virtual void OnEndDraw(){}

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);

	/** Switches the resources so we can have multiple states on this visual.
		The BSP-Tree needs to grab the instancing-buffers for this for example,
		and every node needs its own version */
	virtual void SwitchInstanceSpecificResources(){}

	/** Returns the size of this visual */
	float GetVisualSize();

	/** Returns the visual this is based on */
	zCVisual* GetSourceVisual();
protected:
	/** Original visual from the game */
	zCVisual* SourceVisual;

	/** Size of this visual */
	float VisualSize;
};

