#include "pch.h"
#include "GInstancedStaticMeshVisual.h"
#include "GVobObject.h"
#include "GGame.h"
#include "GWorld.h"
#include "VertexBufferCollection.h"

const int MIN_NUM_INSTANCES_FOR_INSTANCING = 20;

GInstancedStaticMeshVisual::GInstancedStaticMeshVisual(zCVisual* sourceVisual) : GStaticMeshVisual(sourceVisual)
{
	LastFrameDrawn = 0;
	
	InstanceData = NULL;
	InstanceDataInUse = NULL;

	AwaitingUpdate = false;

	NewestStateVersion = 1;
	LastFrameInstances = 0;
}


GInstancedStaticMeshVisual::~GInstancedStaticMeshVisual(void)
{
	Toolbox::DeleteElements(PipelineStates);
	delete InstanceData;
	delete InstanceDataInUse;
}

/** Makes new pipeline states for the given RenderInfo */
void GInstancedStaticMeshVisual::MakePipelineStates(std::vector<PipelineState*>& states, const RenderInfo& info)
{
	if(!IsVisualReady())
		return;

	return;

	AwaitingUpdate = true;

	// Maybe breaks on first frame
	FrameInstanceDataByThread[GetCurrentThreadId()].push_back(&info.CallingVob->GetInstanceInfo());
	
	//m.lock();
	//FrameInstanceData.push_back(&info.CallingVob->GetInstanceInfo());
	//m.unlock();

	if(LastFrameDrawn == Engine::GAPI->GetFrameNum())
	{
		InitFollowingInstances(states);
		return;
	}

	
	RenderMutex.lock();

	if(LastFrameDrawn == Engine::GAPI->GetFrameNum())
	{
		RenderMutex.unlock();
		InitFollowingInstances(states);
		return;
	}

	LastFrameDrawn = Engine::GAPI->GetFrameNum();

	InitFirstInstance(info);

	// First time this is being rendered, set up its pipeline states
	if(states.empty())
		states = PipelineStates;

	RenderMutex.unlock();
}

/** Inits following instances */
void GInstancedStaticMeshVisual::InitFollowingInstances(std::vector<PipelineState*>& states)
{
	// Not the first call to draw this visual, just read the instance info and add it to the instancebuffer
	// Since this is not the first to render, remove any pipelinestates. This is not a memoryleak, since 
	// they are saved in 'PipelineStates' by this class.
	states.clear();

	// Bump the instancecount on every pipelinestate
	for(int i=0;i<PipelineStates.size();i++)
	{
		PipelineStates[i]->NumInstances++;
	}
}

/** Inits the first instance of this frame */
void GInstancedStaticMeshVisual::InitFirstInstance(const RenderInfo& info)
{
	// Make some new states on first render
	if(PipelineStates.empty())
	{
		GStaticMeshVisual::MakePipelineStates(PipelineStates, info);

		// Only use the default transform cb to get around unneccesary statechanges
		std::vector<BaseConstantBuffer*> cbs;
		cbs.push_back(NULL);

		for(int i = 0; i < PipelineStates.size(); i++)
		{
			PipelineStates[i]->SetConstantBuffers(cbs, PipelineState::SS_VERTEX);
		}
	}

	for(int i=0;i<PipelineStates.size();i++)
	{
		// Initialize every state for instanced rendering
		//PipelineStates[i]->BoundCallback = GInstancedStaticMeshVisual::OnStateBoundCallback;
		//PipelineStates[i]->BoundCallbackUserdata = this;
		PipelineStates[i]->NumInstances = 0;

		// Vobs should always ask this visual for a state-update
		PipelineStates[i]->StateVersion = 0;
	}
}

/** Resizes the instancingbuffers if needed. Will allocate a bit more extra-space. */
void GInstancedStaticMeshVisual::ResizeInstancingBuffers(UINT newMinCapacity)
{
	delete InstanceData;
	delete InstanceDataInUse;

	Engine::GraphicsEngine->CreateVertexBuffer(&InstanceData);
	Engine::GraphicsEngine->CreateVertexBuffer(&InstanceDataInUse);

	BaseVertexBuffer* vbs[2] = {InstanceData, InstanceDataInUse};

	// Init our two buffers
	for(int i=0;i<2;i++)
	{
		vbs[i]->Init(NULL, 
			newMinCapacity * 2 * sizeof(VobInstanceInfo),
			BaseVertexBuffer::B_VERTEXBUFFER,
			BaseVertexBuffer::U_DYNAMIC,
			BaseVertexBuffer::CA_WRITE);
	}
}

/** Called when a state of this got bound */
void GInstancedStaticMeshVisual::OnStateBoundCallback(void* userdata)
{

}

/** Called after a renderingpass has been completed. Must be registered in the World for this to be called */
void GInstancedStaticMeshVisual::OnRenderpassEnded()
{
	if(!AwaitingUpdate)
		return;

	// Append our instances to global instance-cache
	VertexBufferCollection<VobInstanceInfo>* instanceCache = Engine::Game->GetWorld()->GetInstancingBufferCollection();

	InstanceBaseLocation = -1;
	for(auto it = FrameInstanceDataByThread.begin(); it != FrameInstanceDataByThread.end(); it++)
	{
		int n = instanceCache->AddDataDeref(&(*it).second[0], (*it).second.size());
		InstanceBaseLocation = InstanceBaseLocation == -1 ? n : InstanceBaseLocation;
	}
	//InstanceBaseLocation = instanceCache->AddDataDeref(&FrameInstanceData[0], FrameInstanceData.size());
}

/** Called right after OnRenderpassEnded has been called on all visuals. */
void GInstancedStaticMeshVisual::OnPostRenderpassEnded()
{
	if(!AwaitingUpdate)
		return;

	VertexBufferCollection<VobInstanceInfo>* instanceCache = Engine::Game->GetWorld()->GetInstancingBufferCollection();

	// Setup instancing-info for all states to come since the buffers could have changed
	for(int i=0;i<PipelineStates.size();i++)
	{
		PipelineStates[i]->SetupInstancing(instanceCache->GetBuffer(), InstanceBaseLocation);

		// Only use the default transform cb to get around unneccesary statechanges
		std::vector<BaseConstantBuffer*> cbs;
		cbs.push_back(NULL);
		PipelineStates[i]->SetConstantBuffers(cbs, PipelineState::SS_VERTEX);
	}

	for(auto it = FrameInstanceDataByThread.begin(); it != FrameInstanceDataByThread.end(); it++)
	{
		(*it).second.clear();
	}
	
	AwaitingUpdate = false;
}