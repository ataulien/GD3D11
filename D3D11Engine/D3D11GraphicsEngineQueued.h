#pragma once
#include "D3D11GraphicsEngineBase.h"




class D3D11GraphicsEngineQueued :
	public D3D11GraphicsEngineBase
{
public:
	D3D11GraphicsEngineQueued(void);
	~D3D11GraphicsEngineQueued(void);
	
	/** Fills the associated state object using the given IDs */
	virtual void FillPipelineStateObject(PipelineState* state);

	/** Draws a single pipeline-state */
	virtual void DrawPipelineState(const PipelineState* state);

	/** Binds a pipeline-state */
	virtual void BindPipelineState(const PipelineState* state);

	/** Fills a pipeline-state with the default value for the current stage */
	virtual void SetupPipelineForStage(int stage, PipelineState* state);

	/** Pushes a single pipeline-state into the renderqueue */
	virtual void PushPipelineState(PipelineState* state);

	/** Flushes the renderqueue */
	virtual void FlushRenderQueue(bool sortQueue = true);

		/** Clears the renderingqueue */
	virtual void ClearRenderingQueue();

	/** Returns the rendering-queue */
	virtual std::vector<PipelineState::PipelineSortItem*>& GetRenderQueue();
protected:

	/** Threadproc for a rendertask */
	static void RenderTask(unsigned int startState, unsigned int numStates);

	/** Binds a pipeline-state without checking the former state*/
	void BindPipelineStateForced(const PipelineState* state);

	/** Renderqueue */
	std::vector<PipelineState::PipelineSortItem*> RenderQueue;

	/** Currently bound pipeline state */
	PipelineState* BoundPipelineState;
	std::unordered_map<unsigned int, PipelineState*> BoundPipelineStateByThread;
	PipelineState* DefaultPipelineState;
};

