#include "pch.h"
#include "D3D11GraphicsEngineQueued.h"
#include "GothicGraphicsState.h"
#include "D3D11PipelineStates.h"
#include "RenderToTextureBuffer.h"
#include "D3D11Texture.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11VertexBuffer.h"

#include "D3D11PShader.h"
#include "D3D11VShader.h"
#include "D3D11HDShader.h"
#include <algorithm>
#include "GothicAPI.h"
#include "ThreadPool.h"

// If this is defined, the BindPipelineState-Method will count statechanges
#define DEBUG_STATECHANGES

#ifdef DEBUG_STATECHANGES
#define SC_DBG(x,s) {x; Engine::GAPI->GetRendererState()->RendererInfo.StateChanges++; Engine::GAPI->GetRendererState()->RendererInfo.StateChangesByState[s]++;}
#else
#define SC_DBG(x,s) {x;}
#endif


D3D11GraphicsEngineQueued::D3D11GraphicsEngineQueued(void)
{
	BoundPipelineState = NULL;
	DefaultPipelineState = CreatePipelineState();
	memset(DefaultPipelineState, 0xFF, sizeof(D3D11PipelineState)); // Init all bytes with 0xFF, so we have to rebind everything in case this gets bound

	BoundPipelineState = DefaultPipelineState;
}


D3D11GraphicsEngineQueued::~D3D11GraphicsEngineQueued(void)
{
	delete DefaultPipelineState;
}

/** Draws a single pipeline-state */
void D3D11GraphicsEngineQueued::DrawPipelineState(const PipelineState* state)
{
	if (Engine::GAPI->GetRendererState()->RendererSettings.DisableDrawcalls)
		return;

	ID3D11DeviceContext* context = GetDeferredContextByThread();

	switch(state->BaseState.DrawCallType)
	{
	case PipelineState::DCT_DrawTriangleList:
		context->Draw(state->BaseState.NumVertices, 0);
		break;

	case PipelineState::DCT_DrawIndexed:
		context->DrawIndexed(state->BaseState.NumIndices, state->BaseState.IndexOffset, 0);
		break;

	case PipelineState::DCT_DrawIndexedInstanced:
		context->DrawIndexedInstanced(state->BaseState.NumIndices, state->BaseState.NumInstances, 0, 0, state->BaseState.InstanceOffset);
		break;
	}
}

/** Binds a pipeline-state without checking the former state*/
void D3D11GraphicsEngineQueued::BindPipelineStateForced(const PipelineState* state)
{
	D3D11PipelineState* s = (D3D11PipelineState*)state;

	// Bind state
	Context->OMSetBlendState(s->BlendState,  (float *)&D3DXVECTOR4(0, 0, 0, 0), 0xFFFFFFFF);
	Context->PSSetSamplers(0, 1, &s->SamplerState);
	Context->OMSetDepthStencilState(s->DepthStencilState, 0);
	Context->RSSetState(s->RasterizerState);

	// Bind constantbuffers
	Context->VSSetConstantBuffers(0, s->ConstantBuffersVS.size(), &s->ConstantBuffersVS[0]);
	Context->PSSetConstantBuffers(0, s->ConstantBuffersPS.size(), &s->ConstantBuffersPS[0]);
	Context->HSSetConstantBuffers(0, s->ConstantBuffersHDS.size(), &s->ConstantBuffersHDS[0]);
	Context->DSSetConstantBuffers(0, s->ConstantBuffersHDS.size(), &s->ConstantBuffersHDS[0]);
	Context->GSSetConstantBuffers(0, s->ConstantBuffersGS.size(), &s->ConstantBuffersGS[0]);

	// Vertexbuffers
	UINT off[] = {0,0};
	Context->IASetVertexBuffers(0, s->VertexBuffers.size(), &s->VertexBuffers[0], s->BaseState.VertexStride, off);
	Context->IASetIndexBuffer(s->IndexBuffer, s->BaseState.IndexStride == 32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, s->BaseState.IndexOffset);

	// Shaders
	Context->VSSetShader(s->VertexShader, NULL, NULL);
	Context->IASetInputLayout(s->InputLayout);
	Context->PSSetShader(s->PixelShader, NULL, NULL);
	Context->HSSetShader(s->HullShader, NULL, NULL);
	Context->DSSetShader(s->DomainShader, NULL, NULL);
	Context->GSSetShader(s->GeometryShader, NULL, NULL);

	// Rendertargets
	Context->OMSetRenderTargets(8, s->RenderTargetViews, s->DepthStencilView);

	// Textures
	Context->PSSetShaderResources(0, 8, s->Textures);

	// Primitive topology
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#ifdef DEBUG_STATECHANGES
	Engine::GAPI->GetRendererState()->RendererInfo.StateChanges+=19;
#endif
}

/** Binds a pipeline-state */
void D3D11GraphicsEngineQueued::BindPipelineState(const PipelineState* state)
{
	D3D11PipelineState* s = (D3D11PipelineState*)state;
	D3D11PipelineState* b = (D3D11PipelineState*)BoundPipelineStateByThread[GetCurrentThreadId()];
	ID3D11DeviceContext* context = GetDeferredContextByThread();

	if(!b) b = (D3D11PipelineState*)&DefaultPipelineState;

	// Bind state
	if(b->BlendState != s->BlendState)
		SC_DBG(context->OMSetBlendState(s->BlendState,  (float *)&D3DXVECTOR4(0, 0, 0, 0), 0xFFFFFFFF), GothicRendererInfo::SC_BS);

	if(b->SamplerState != s->SamplerState)
		SC_DBG(context->PSSetSamplers(0, 1, &s->SamplerState), GothicRendererInfo::SC_SMPL);

	if(b->DepthStencilState != s->DepthStencilState)
		SC_DBG(context->OMSetDepthStencilState(s->DepthStencilState, 0), GothicRendererInfo::SC_DSS);

	if(b->RasterizerState != s->RasterizerState)
		SC_DBG(context->RSSetState(s->RasterizerState), GothicRendererInfo::SC_RS);

	// Bind constantbuffers (They are likely to change for every object)
	if(!s->ConstantBuffersVS.empty() && s->ConstantBuffersVS != b->ConstantBuffersVS)SC_DBG(context->VSSetConstantBuffers(0, s->ConstantBuffersVS.size(), &s->ConstantBuffersVS[0]),GothicRendererInfo::SC_CB);
	if(!s->ConstantBuffersPS.empty() && s->ConstantBuffersPS != b->ConstantBuffersPS)SC_DBG(context->PSSetConstantBuffers(0, s->ConstantBuffersPS.size(), &s->ConstantBuffersPS[0]),GothicRendererInfo::SC_CB);
	if(!s->ConstantBuffersHDS.empty() && s->ConstantBuffersHDS != b->ConstantBuffersHDS)SC_DBG(context->HSSetConstantBuffers(0, s->ConstantBuffersHDS.size(), &s->ConstantBuffersHDS[0]),GothicRendererInfo::SC_CB);
	if(!s->ConstantBuffersHDS.empty() && s->ConstantBuffersHDS != b->ConstantBuffersHDS)SC_DBG(context->DSSetConstantBuffers(0, s->ConstantBuffersHDS.size(), &s->ConstantBuffersHDS[0]),GothicRendererInfo::SC_CB);
	if(!s->ConstantBuffersGS.empty() && s->ConstantBuffersGS != b->ConstantBuffersGS)SC_DBG(context->GSSetConstantBuffers(0, s->ConstantBuffersGS.size(), &s->ConstantBuffersGS[0]),GothicRendererInfo::SC_CB);

	// Vertexbuffers
	UINT off[] = {0,0};
	if(memcmp(s->BaseState.VertexBuffers, b->BaseState.VertexBuffers, sizeof(b->BaseState.VertexBuffers)) != 0)
		SC_DBG(context->IASetVertexBuffers(0, s->VertexBuffers.size(), &s->VertexBuffers[0], s->BaseState.VertexStride, off), GothicRendererInfo::SC_VB);

	if(!s->StructuredBuffersVS.empty() && memcmp(s->BaseState.StructuredBuffersVS, b->BaseState.StructuredBuffersVS, sizeof(b->BaseState.StructuredBuffersVS)) != 0)
		SC_DBG(context->VSSetShaderResources(0, 1, &s->StructuredBuffersVS[0]), GothicRendererInfo::SC_VB);

	if(s->IndexBuffer != b->IndexBuffer)
		SC_DBG(context->IASetIndexBuffer(s->IndexBuffer, s->BaseState.IndexStride == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0), GothicRendererInfo::SC_IB);

	// Shaders
	if(s->VertexShader != b->VertexShader)
		SC_DBG(context->VSSetShader(s->VertexShader, NULL, NULL), GothicRendererInfo::SC_VS);
	
	if(s->InputLayout != b->InputLayout)
		SC_DBG(context->IASetInputLayout(s->InputLayout), GothicRendererInfo::SC_IL);
	
	if(s->PixelShader != b->PixelShader)
		SC_DBG(context->PSSetShader(s->PixelShader, NULL, NULL), GothicRendererInfo::SC_PS);
	
	if(s->HullShader != b->HullShader)
		SC_DBG(context->HSSetShader(s->HullShader, NULL, NULL), GothicRendererInfo::SC_HS);
	
	if(s->DomainShader != b->DomainShader)
		SC_DBG(context->DSSetShader(s->DomainShader, NULL, NULL), GothicRendererInfo::SC_DS);
	
	if(s->GeometryShader != b->GeometryShader)
		SC_DBG(context->GSSetShader(s->GeometryShader, NULL, NULL), GothicRendererInfo::SC_GS);

	// Rendertargets
	if(memcmp(s->RenderTargetViews, b->RenderTargetViews, sizeof(void*) * s->NumRenderTargetViews) != 0 ||
		s->DepthStencilView != b->DepthStencilView)
		SC_DBG(context->OMSetRenderTargets(s->NumRenderTargetViews, s->RenderTargetViews, s->DepthStencilView), GothicRendererInfo::SC_RTVDSV);

	// Textures
	if(memcmp(s->Textures, b->Textures, sizeof(void*) * s->BaseState.NumTextures) != 0)
		SC_DBG(context->PSSetShaderResources(0, s->BaseState.NumTextures, s->Textures), GothicRendererInfo::SC_TX);

	// Primitive topology
	//Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Replace old state
	// FIXME: Might not threadsave
	BoundPipelineStateByThread[GetCurrentThreadId()] = s;
}
/** Fills the associated state object using the given IDs */
void D3D11GraphicsEngineQueued::FillPipelineStateObject(PipelineState* state)
{
	D3D11PipelineState* s = (D3D11PipelineState*)state;

	// Get state objects from id
	D3D11BlendStateInfo* blendState = D3D11ObjectIDs::BlendStateByID[(UINT8)state->BaseState.BlendStateID];
	D3D11RasterizerStateInfo* rasterizerState = D3D11ObjectIDs::RasterizerStateByID[(UINT8)state->BaseState.RasterizerStateID];
	D3D11DepthBufferState* depthStencilState = D3D11ObjectIDs::DepthStateByID[(UINT8)state->BaseState.DepthStencilID];

	// Put them into our queue object
	if(blendState)
		s->BlendState = blendState->State;

	if(rasterizerState)
		s->RasterizerState = rasterizerState->State;

	if(depthStencilState)
		s->DepthStencilState = depthStencilState->State;

	// TODO: Do this for sampler states as well
	s->SamplerState = DefaultSamplerState;

	// Enter rendertargets
	s->RenderTargetViews[0] = Backbuffer->GetRenderTargetView();
	s->NumRenderTargetViews = 1;
	s->DepthStencilView = DepthStencilBuffer->GetDepthStencilView();

	// Enter textures
	for(int i=0;i<8;i++)
	{
		D3D11Texture* tex = (D3D11Texture *)D3D11ObjectIDs::TextureByID[s->BaseState.TextureIDs[i]];
		if(!tex)
			break;

		s->Textures[i] = tex->GetShaderResourceView();
		s->BaseState.NumTextures = i+1;
	}

	// Buffers
	s->ConstantBuffersVS.clear();
	for(int i=0;i<ARRAYSIZE(s->BaseState.ConstantBuffersVS);i++)
	{
		D3D11ConstantBuffer* buf = (D3D11ConstantBuffer *)s->BaseState.ConstantBuffersVS[i];
		if(!buf)
			break;

		s->ConstantBuffersVS.push_back(buf->Get());
	}

	s->ConstantBuffersPS.clear();
	for(int i=0;i<ARRAYSIZE(s->BaseState.ConstantBuffersPS);i++)
	{
		D3D11ConstantBuffer* buf = (D3D11ConstantBuffer *)s->BaseState.ConstantBuffersPS[i];
		if(!buf)
			break;

		s->ConstantBuffersPS.push_back(buf->Get());
	}

	s->ConstantBuffersHDS.clear();
	for(int i=0;i<ARRAYSIZE(s->BaseState.ConstantBuffersHDS);i++)
	{
		D3D11ConstantBuffer* buf = (D3D11ConstantBuffer *)s->BaseState.ConstantBuffersHDS[i];
		if(!buf)
			break;

		s->ConstantBuffersHDS.push_back(buf->Get());
	}

	s->ConstantBuffersGS.clear();
	for(int i=0;i<ARRAYSIZE(s->BaseState.ConstantBuffersGS);i++)
	{
		D3D11ConstantBuffer* buf = (D3D11ConstantBuffer *)s->BaseState.ConstantBuffersGS[i];
		if(!buf)
			break;

		s->ConstantBuffersGS.push_back(buf->Get());
	}

	s->VertexBuffers.clear();
	for(int i=0;i<ARRAYSIZE(s->BaseState.VertexBuffers);i++)
	{
		D3D11VertexBuffer* buf = (D3D11VertexBuffer *)s->BaseState.VertexBuffers[i];
		if(buf)
			s->VertexBuffers.push_back(buf->GetVertexBuffer());
	}

	s->StructuredBuffersVS.clear();
	for(int i=0;i<ARRAYSIZE(s->BaseState.StructuredBuffersVS);i++)
	{
		D3D11VertexBuffer* buf = (D3D11VertexBuffer *)s->BaseState.StructuredBuffersVS[i];
		if(buf)
			s->StructuredBuffersVS.push_back(buf->GetShaderResourceView());
	}

	if(s->BaseState.IndexBuffer)
		s->IndexBuffer = ((D3D11VertexBuffer *)s->BaseState.IndexBuffer)->GetVertexBuffer();

	D3D11PShader* ps = D3D11ObjectIDs::PShadersByID[s->BaseState.PShaderID];
	D3D11VShader* vs = D3D11ObjectIDs::VShadersByID[s->BaseState.VShaderID];
	D3D11HDShader* hds = D3D11ObjectIDs::HDShadersByID[s->BaseState.HDShaderID];
	//D3D11PShader* gs = NULL;//D3D11PShader::ShadersByID[s->BaseState.PShaderID];


	s->PixelShader = ps ? ps->GetShader() : NULL;
	s->VertexShader = vs ? vs->GetShader() : NULL;
	s->InputLayout = vs ? vs->GetInputLayout() : NULL;
	s->HullShader = hds ? hds->GetHShader() : NULL;
	s->DomainShader = hds ? hds->GetDShader() : NULL;
	s->GeometryShader = NULL;//D3D11PShader::ShadersByID[s->BaseState.PShaderID]->GetShader();

	// Build the sort-key
	s->SortItem.stateValue = s->SortItem.Build(s->BaseState);
}

/** Fills a pipeline-state with the default value for the current stage */
void D3D11GraphicsEngineQueued::SetupPipelineForStage(int stage, PipelineState* state)
{
	state->BaseState.ConstantBuffersVS[0] = TransformsCB;
	state->BaseState.VertexStride[0] = sizeof(ExVertexStruct);
	state->BaseState.IndexStride = sizeof(VERTEX_INDEX);

	// Handle alpha-mask
	if(state->BaseState.TranspacenyMode == PipelineState::ETransparencyMode::TM_MASKED)
		state->BaseState.PShaderID = PS_SimpleAlphaTest->GetID();
	else
		state->BaseState.PShaderID = PS_Simple->GetID();

	if(stage == STAGE_DRAW_WORLD)
	{
		if(state->BaseState.DrawCallType != PipelineState::DCT_DrawIndexedInstanced)
			state->BaseState.VShaderID = VS_Ex->GetID();
		else
			state->BaseState.VShaderID = VS_ExRemapInstancedObj->GetID();
	}else if(stage == STAGE_DRAW_SKELETAL)
	{
		state->BaseState.VertexStride[0] = sizeof(ExSkelVertexStruct);
		state->BaseState.VShaderID = VS_ExSkeletal->GetID();
	}
}

/** Pushes a single pipeline-state into the renderqueue */
void D3D11GraphicsEngineQueued::PushPipelineState(PipelineState* state)
{
	static std::mutex m;

	m.lock();
	RenderQueue.push_back(&state->SortItem);
	m.unlock();
}

/** Threadproc for a rendertask */
void D3D11GraphicsEngineQueued::RenderTask(unsigned int startState, unsigned int numStates)
{
	D3D11GraphicsEngineQueued* engine = (D3D11GraphicsEngineQueued *)Engine::GraphicsEngine;

	for(int i = startState; i < startState + numStates; i++)
	{
		engine->BindPipelineState(engine->RenderQueue[i]->AssociatedState);
		engine->DrawPipelineState(engine->RenderQueue[i]->AssociatedState);
	}

	for(int i = startState; i < startState + numStates; i++)
	{
		engine->RenderQueue[i]->AssociatedState->StateWasDrawn();
	}
}

/** Flushes the renderqueue */
void D3D11GraphicsEngineQueued::FlushRenderQueue(bool sortQueue)
{
	BoundPipelineState = DefaultPipelineState;

	// Execute the commandlists from our worker-threads
	ExecuteDeferredCommandLists();

	// Initial clear
	Clear(Engine::GAPI->GetRendererState()->GraphicsState.FF_FogColor);

	// Force farplane
	Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);

	// Primitive topology
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if(!Engine::GAPI->GetRendererState()->RendererSettings.DisableRendering)
	{
		Engine::GAPI->GetRendererState()->RendererInfo.FramePipelineStates = RenderQueue.size();

		if(Engine::GAPI->GetRendererState()->RendererSettings.SortRenderQueue)
		{
			// Sort the queue, if wanted
			if(sortQueue)
				std::sort(RenderQueue.begin(), RenderQueue.end(), PipelineState::PipelineSortItem::cmp);

			if(Engine::GAPI->GetRendererState()->RendererSettings.DrawThreaded)
			{
				// Figure out how much work there is to do for each thread
				int n = RenderQueue.size() / Engine::RenderingThreadPool->getNumThreads();
				int rest = RenderQueue.size() - n * Engine::RenderingThreadPool->getNumThreads();

				// If there are less states than threads, just draw them
				if(n == 0)
				{
					// Draw all states
					for(auto it = RenderQueue.begin(); it != RenderQueue.end(); it++)
					{
						BindPipelineState((*it)->AssociatedState);
						DrawPipelineState((*it)->AssociatedState);

						(*it)->AssociatedState->StateWasDrawn();
						//testset.insert((*it));
					}
				}
				else
				{
					std::vector<std::future<void>> futures;
					for(int i = 0; i < Engine::RenderingThreadPool->getNumThreads(); i++)
					{
						std::future<void> future;
						// Enqueue threads
						if(i == Engine::RenderingThreadPool->getNumThreads() - 1)
							futures.push_back(Engine::RenderingThreadPool->enqueue(D3D11GraphicsEngineQueued::RenderTask, i*n, n + rest)); // last thread simply gets the remainder
						else
							futures.push_back(Engine::RenderingThreadPool->enqueue(D3D11GraphicsEngineQueued::RenderTask, i*n, n));
					}

					// Now wait for all of them
					for(int i = 0; i < futures.size(); i++)
					{
						futures[i].wait();
						// TODO: Could already execute that contexts commandlist here!
					}

					ExecuteDeferredCommandLists();
				}
			}
			else
			{
				// Draw all states
				for(auto it = RenderQueue.begin(); it != RenderQueue.end(); it++)
				{
					BindPipelineState((*it)->AssociatedState);
					DrawPipelineState((*it)->AssociatedState);

					(*it)->AssociatedState->StateWasDrawn();
					//testset.insert((*it));
				}
			}
			
			//std::set<PipelineState::PipelineSortItem*> testset;

			// Draw all states
			/*for(auto it = RenderQueue.begin(); it != RenderQueue.end(); it++)
			{
				BindPipelineState((*it)->AssociatedState);
				DrawPipelineState((*it)->AssociatedState);

				(*it)->AssociatedState->StateWasDrawn();
				//testset.insert((*it));
			}*/

			//if(testset.size() != RenderQueue.size())
			//	LogWarn() << "Renderer: Submitted one pipelinestate more than once!";

		}else
		{
			// Draw all states
			for(auto it = RenderQueue.begin(); it != RenderQueue.end(); it++)
			{
				BindPipelineStateForced((*it)->AssociatedState);
				DrawPipelineState((*it)->AssociatedState);

				(*it)->AssociatedState->StateWasDrawn();
			}

		
		}
	}

	// Clear the renderqueue for next stage
	ClearRenderingQueue();

	SetDefaultStates();
}

/** Clears the renderingqueue */
void D3D11GraphicsEngineQueued::ClearRenderingQueue()
{
	RenderQueue.clear();
}

/** Returns the rendering-queue */
std::vector<PipelineState::PipelineSortItem*>& D3D11GraphicsEngineQueued::GetRenderQueue()
{
	return RenderQueue;
}