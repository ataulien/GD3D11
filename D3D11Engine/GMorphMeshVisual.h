#pragma once
#include "GVisual.h"

class GMorphMeshVisual : public GVisual
{
public:
	GMorphMeshVisual(zCVisual* sourceVisual);
	~GMorphMeshVisual(void);

	/** Draws the visual for the given vob */
	virtual void DrawVisual(const RenderInfo& info);
	
	/** Called on a new frame */
	virtual void OnBeginDraw();

	/** Called when we are done drawing */
	virtual void OnEndDraw();

protected:
	/** Returns a fresh usable cache-slot. Trys to reuse old vertexbuffers, but will create a new one
		if needed. After called, you can use the VertexBufferCache for your submesh at the returned index. */
	int RequestCacheVB();

	/** Updates vertexbuffers in the cache at the given slot */
	void UpdateVertexbuffers(int slot, float fatness = 0);

	/** Vertexbuffercache for all currently active instances of this, sorted by submesh */
	std::vector<std::vector<BaseVertexBuffer*>> VertexBufferCache;

	/** Current position of the cache */
	unsigned int CacheIndex;

	/** Pipelinestate for this part */
	std::vector<PipelineState*> ImmediatePipelineStates;
};

