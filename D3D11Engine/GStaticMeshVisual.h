#pragma once
#include "gvisual.h"
#include "WorldConverter.h"
#include "BaseGraphicsEngine.h"

struct MeshInfo;
struct MeshVisualInfo;


/** Visual-Wrapper for zCProgMeshProto */
class GStaticMeshVisual :
	public GVisual
{
public:
	GStaticMeshVisual(zCVisual* sourceVisual);
	~GStaticMeshVisual(void);

	/** Called on a new frame */
	virtual void OnBeginDraw();

	/** Called when we are done drawing */
	virtual void OnEndDraw();

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);

	/** Switches the resources so we can have multiple states on this visual.
		The BSP-Tree needs to grab the instancing-buffers for this for example,
		and every node needs its own version */
	virtual void SwitchInstanceSpecificResources();

protected:

	/** Registers an instance */
	void RegisterInstance(const RenderInfo& info);

	/** Draws the instances registered in this buffer */
	virtual void DrawInstances();

	/** Doubles the size the instancing buffer can hold (Recreates buffer) */
	XRESULT IncreaseInstancingBufferSize();

	/** Structure that holds mesh information about this visual */
	MeshVisualInfo VisualInfo;

	/** Pipelinestate to render this with */
	std::vector<PipelineState*> PipelineStates;

	struct {
		/** Instancingbuffer of this visual */
		BaseVertexBuffer* InstancingBuffer;
		
		/** Number of registered instances after the last OnBeginDraw-Call */
		unsigned int NumRegisteredInstances;

		/** Instancingdata of this frame */
		std::vector<VobInstanceInfo> FrameInstanceData;

		/** Data of the InstancingBuffer. This is only valid between OnBeginDraw and OnEndDraw and when
			InstanceBufferData is greater than 0 */
		byte* InstancingBufferData;
	} Instancing;
};

