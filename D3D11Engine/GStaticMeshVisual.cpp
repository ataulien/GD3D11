#include "pch.h"
#include "GStaticMeshVisual.h"
#include "WorldConverter.h"
#include "Engine.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"
#include "GVobObject.h"
#include "zCVob.h"
#include "zCMaterial.h"

const int INSTANCING_BUFFER_SIZE = sizeof(VobInstanceInfo) * 64; // TODO: This is too small! But higher values are too memory intensive!

GStaticMeshVisual::GStaticMeshVisual(zCVisual* sourceVisual) : GVisual(sourceVisual)
{
	// Extract the info from the visual in the games memory
	WorldConverter::Extract3DSMeshFromVisual2((zCProgMeshProto*)sourceVisual, &VisualInfo);

	// Set other variables to their initial value
	Instancing.NumRegisteredInstances = 0;
	VisualSize = VisualInfo.MeshSize;

	// Create pipelinestates and instancing buffer
	SwitchInstanceSpecificResources();
}


GStaticMeshVisual::~GStaticMeshVisual(void)
{
	Toolbox::DeleteElements(PipelineStates);

	delete Instancing.InstancingBuffer;
}

/** Switches the resources so we can have multiple states on this visual.
		The BSP-Tree needs to grab the instancing-buffers for this for example,
		and every node needs its own version */
void GStaticMeshVisual::SwitchInstanceSpecificResources()
{
	if(Instancing.NumRegisteredInstances != 0)
	{
		LogWarn() << "SwitchInstanceSpecificResources on mapped instancing buffer!";
		return;
	}

	// Create the instancing buffer for this visual
	// Someone else now needs to take care of deleting this buffer
	Engine::GraphicsEngine->CreateVertexBuffer(&Instancing.InstancingBuffer);

	// Init it
	Instancing.InstancingBuffer->Init(NULL, INSTANCING_BUFFER_SIZE, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);

	// Someone else needs to take care of deleting the memory of these now
	PipelineStates.clear();

	// We need as many pipeline states as we have textures in the mesh
	for(auto it = VisualInfo.Meshes.begin();it != VisualInfo.Meshes.end(); it++)
	{
		for(int i = 0;i<(*it).second.size();i++)
		{
			// Enter a new state for this submesh-part
			PipelineStates.push_back(Engine::GraphicsEngine->CreatePipelineState());

			PipelineStates.back()->BaseState.DrawCallType = PipelineState::DCT_DrawIndexedInstanced;

			Engine::GraphicsEngine->SetupPipelineForStage(STAGE_DRAW_WORLD, PipelineStates.back());
				
			PipelineStates.back()->BaseState.VertexBuffers[0] = (*it).second[i]->MeshVertexBuffer;
			PipelineStates.back()->BaseState.IndexBuffer = (*it).second[i]->MeshIndexBuffer;
			PipelineStates.back()->BaseState.NumIndices = (*it).second[i]->Indices.size();
			PipelineStates.back()->BaseState.NumVertices = (*it).second[i]->Vertices.size();


			// Huge safety-check to see if gothic didn't mess this up
			if((*it).first &&
				(*it).first->GetTexture() &&
				(*it).first->GetTexture()->GetSurface() &&
				(*it).first->GetTexture()->GetSurface()->GetEngineTexture())
				PipelineStates.back()->BaseState.TextureIDs[0] = (*it).first->GetTexture()->GetSurface()->GetEngineTexture()->GetID();

			// Enter API-Specific values into the state-object
			Engine::GraphicsEngine->FillPipelineStateObject(PipelineStates.back());
		}
	}
}

/** Registers an instance */
void GStaticMeshVisual::RegisterInstance(const RenderInfo& info)
{
	// Add the world-matrix of this vob to the instancing buffer
	unsigned int arrayPos = Instancing.NumRegisteredInstances * sizeof(VobInstanceInfo);

	// If the buffer is too small, resize it
	if(arrayPos + sizeof(VobInstanceInfo) > Instancing.InstancingBuffer->GetSizeInBytes())
	{
		IncreaseInstancingBufferSize();
	}

	// Check if this is the first instance we need to register here
	if(Instancing.NumRegisteredInstances == 0)
	{
		// Since we have at least one instance to fill, map the buffer now and save the datapointer
		UINT size;
		XLE(Instancing.InstancingBuffer->Map(BaseVertexBuffer::M_WRITE_DISCARD, (void**)&Instancing.InstancingBufferData, &size));

		int s = 0;
		for(auto it = VisualInfo.Meshes.begin();it != VisualInfo.Meshes.end(); it++)
		{
			std::vector<MeshInfo*>& meshes = (*it).second;
			for(int i=0;i<meshes.size();i++)
			{

				// Huge safety-check to see if gothic didn't mess this up
				if(PipelineStates[s]->BaseState.TextureIDs[0] == 0xFFFF ||
					PipelineStates[s]->BaseState.ConstantBuffersVS[1] == NULL)
				{

					// Only draw if the texture is loaded
					if((*it).first->GetTexture() && (*it).first->GetTexture()->CacheIn(0.6f) != zRES_CACHED_IN)
					{
						//s++;
						//continue;
						PipelineStates[s]->BaseState.TextureIDs[0] = 0;
					}

					// Get texture ID if everything is allright
					if((*it).first &&
						(*it).first->GetTexture() &&
						(*it).first->GetTexture()->GetSurface() &&
						(*it).first->GetTexture()->GetSurface()->GetEngineTexture())
						PipelineStates[s]->BaseState.TextureIDs[0] = (*it).first->GetTexture()->GetSurface()->GetEngineTexture()->GetID();
				}

				// Give our instancingbuffer to the state
				PipelineStates[s]->BaseState.VertexBuffers[1] = Instancing.InstancingBuffer;
				PipelineStates[s]->BaseState.VertexStride[1] = sizeof(VobInstanceInfo);
				Engine::GraphicsEngine->FillPipelineStateObject(PipelineStates[s]);

				PipelineStates[s]->BaseState.NumInstances = 0;

				// Register this state, instances are added afterwards
				Engine::GraphicsEngine->PushPipelineState(PipelineStates[s]);
				s++;
			}
		}
	}

	if(!Instancing.InstancingBufferData)
		return; // Failed to map?

	// arrayPos is valid, add the instance
	memcpy((Instancing.InstancingBufferData) + arrayPos, &info.CallingVob->GetInstanceInfo(), sizeof(VobInstanceInfo));
	Instancing.NumRegisteredInstances++;

	// Save, in case we have to recreate the buffer. // TODO: This is only a temporary solution for the BSP-Pre-Draw
	Instancing.FrameInstanceData.push_back(info.CallingVob->GetInstanceInfo());

	int s = 0;
	for(auto it = VisualInfo.Meshes.begin();it != VisualInfo.Meshes.end(); it++)
	{
		std::vector<MeshInfo*>& meshes = (*it).second;
		for(int i=0;i<meshes.size();i++)
		{
			PipelineStates[s]->BaseState.NumInstances = Instancing.NumRegisteredInstances;
			s++;
		}
	}
}


/** Draws the visual for the given vob */
void GStaticMeshVisual::DrawVisual(const RenderInfo& info)
{
	RegisterInstance(info);
	return;

	// Make sure the vob has enought slots for pipeline states
	if(info.CallingVob->GetPipelineStates().size() < PipelineStates.size())
		info.CallingVob->GetPipelineStates().resize(PipelineStates.size());

	// Set up states and draw mesh
	int s=0;
	for(auto it = VisualInfo.Meshes.begin();it != VisualInfo.Meshes.end(); it++)
	{
		D3DXMATRIX world; D3DXMatrixTranspose(&world, info.WorldMatrix);

		std::vector<MeshInfo*>& meshes = (*it).second;
		for(int i=0;i<meshes.size();i++)
		{
#ifndef PUBLIC_RELEASE
			if(s >= PipelineStates.size())
			{
				LogError() << "GStaticMeshVisual needs more pipeline-states than available!";
				break;
			}
#endif

			// Huge safety-check to see if gothic didn't mess this up
			if(PipelineStates[s]->BaseState.TextureIDs[0] == 0xFFFF ||
				PipelineStates[s]->BaseState.ConstantBuffersVS[1] == NULL)
			{

				// Only draw if the texture is loaded
				if((*it).first->GetTexture() && (*it).first->GetTexture()->CacheIn(0.6f) != zRES_CACHED_IN)
				{
					s++;
					continue;
				}

				// Get texture ID if everything is allright
				if((*it).first &&
					(*it).first->GetTexture() &&
					(*it).first->GetTexture()->GetSurface() &&
					(*it).first->GetTexture()->GetSurface()->GetEngineTexture())
					PipelineStates[s]->BaseState.TextureIDs[0] = (*it).first->GetTexture()->GetSurface()->GetEngineTexture()->GetID();
			}

			// Check if we need to add the current state to the vobs state-cache
			if(!info.CallingVob->GetPipelineStates()[s])
			{
				// Clone pipeline state to the vob
				info.CallingVob->GetPipelineStates()[s] = Engine::GraphicsEngine->CreatePipelineState(PipelineStates[s]);

				// Insert the vobs constantbuffer
				info.CallingVob->GetPipelineStates()[s]->BaseState.ConstantBuffersVS[1] = info.InstanceCB;
				Engine::GraphicsEngine->FillPipelineStateObject(info.CallingVob->GetPipelineStates()[s]);
			}
			

			// Put distance into the state
			//info.CallingVob->GetPipelineStates()[s]->SortItem.state.Depth = info.Distance > 0 ? std::min((UINT)info.Distance, (UINT)0xFFFFFF) : 0xFFFFFF;
			//info.CallingVob->GetPipelineStates()[s]->SortItem.state.Build(info.CallingVob->GetPipelineStates()[s]->BaseState);

			// Push state to render-queue
			Engine::GraphicsEngine->PushPipelineState(info.CallingVob->GetPipelineStates()[s]);

			//Engine::GraphicsEngine->BindPipelineState(PipelineStates[s]);
			//Engine::GraphicsEngine->DrawPipelineState(PipelineStates[s]);
			//Engine::GraphicsEngine->GetLineRenderer()->AddWireframeMesh(meshes[i]->Vertices, meshes[i]->Indices, D3DXVECTOR4(0,1,0,1), &world);

			s++;
		}
	}
}


/** Called on a new frame */
void GStaticMeshVisual::OnBeginDraw()
{
	Instancing.NumRegisteredInstances = 0;
	Instancing.FrameInstanceData.clear();
}

/** Called when we are done drawing */
void GStaticMeshVisual::OnEndDraw()
{
	// Check if we actually had something to do this frame
	if(Instancing.NumRegisteredInstances != 0)
	{
		// Yes! This means that the instancing-buffer currently is mapped.
		// Unmap it now.
		XLE(Instancing.InstancingBuffer->Unmap());
		Instancing.InstancingBufferData = NULL;

		Instancing.NumRegisteredInstances = 0;
		Instancing.FrameInstanceData.clear();
	}
}

/** Draws the instances registered in this buffer */
void GStaticMeshVisual::DrawInstances()
{

}

/** Doubles the size the instancing buffer can hold (Recreates buffer) */
XRESULT GStaticMeshVisual::IncreaseInstancingBufferSize()
{
	// Unmap, in case we are mapped
	if(Instancing.InstancingBufferData)
	{
		XLE(Instancing.InstancingBuffer->Unmap());
		Instancing.InstancingBufferData = NULL;
	}

	// Increase size
	unsigned int size = Instancing.InstancingBuffer->GetSizeInBytes();

	// Recreate the buffer
	delete Instancing.InstancingBuffer;
	Engine::GraphicsEngine->CreateVertexBuffer(&Instancing.InstancingBuffer);

	// Create a new buffer with the size doubled
	XLE(Instancing.InstancingBuffer->Init(NULL, size * 2, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE));

	// Fill old data
	//Instancing.InstancingBuffer->UpdateBuffer(&Instancing.FrameInstanceData[0], Instancing.FrameInstanceData.size() * sizeof(VobInstanceInfo));

	// FIXME: This might leads to some objects disappearing for one frame because the buffer got emptied, but I don't
	// want to write a full second array of instance-infos just because of that.
	//Instancing.NumRegisteredInstances = 0;

	return XR_SUCCESS;
}