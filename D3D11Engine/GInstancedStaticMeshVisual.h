#pragma once
#include "GStaticMeshVisual.h"

class GInstancedStaticMeshVisual : public GStaticMeshVisual
{
public:
	GInstancedStaticMeshVisual(zCVisual* sourceVisual);
	~GInstancedStaticMeshVisual(void);

	/** Makes new pipeline states for the given RenderInfo */
	virtual void MakePipelineStates(std::vector<PipelineState*>& states, const RenderInfo& info);

	/** Called after a renderingpass has been completed. Must be registered in the World for this to be called */
	virtual void OnRenderpassEnded();

	/** Called right after OnRenderpassEnded has been called on all visuals. */
	virtual void OnPostRenderpassEnded();
protected:
	/** Resizes the instancingbuffers if needed. Will allocate a bit more extra-space. */
	void ResizeInstancingBuffers(UINT newMinCapacity);

	/** Called when a state of this got bound */
	static void OnStateBoundCallback(void* userdata);

	/** Inits the first instance of this frame */
	void InitFirstInstance(const RenderInfo& info);

	/** Inits following instances */
	void InitFollowingInstances(std::vector<PipelineState*>& states);

	/** Framenumber this was drawn the last time */
	UINT LastFrameDrawn;

	/** Instances rendered the last time. Just go with usual rendering if this is low. */
	UINT LastFrameInstances;

	/** Contains all of the rendered instance-data */
	std::vector<const VobInstanceInfo*> FrameInstanceData;
	std::map<int, std::vector<const VobInstanceInfo*>> FrameInstanceDataByThread;

	/** Pipelinestates for the batch */
	std::vector<PipelineState*> PipelineStates;

	/** Instancedata buffers */
	BaseVertexBuffer* InstanceData;
	BaseVertexBuffer* InstanceDataInUse; // Doublebuffered
	bool AwaitingUpdate;

	/** Instance base location in instancecollection */
	unsigned int InstanceBaseLocation;

	/** Rendermutex for this visual */
	std::mutex RenderMutex;
};

