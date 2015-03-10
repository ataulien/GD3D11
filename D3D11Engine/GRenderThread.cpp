#include "pch.h"
#include "GRenderThread.h"
#include <thread>
#include <mutex>

#include "BaseConstantBuffer.h"
#include "WorldConverter.h"
#include "Engine.h"
#include "BaseGraphicsEngine.h"
#include "BaseTexture.h"

GRenderThread::GRenderThread(void)
{
	PendingFrameInfo = NULL;
	WorkingFrameInfo = NULL;
}


GRenderThread::~GRenderThread(void)
{
	delete PendingFrameInfo;
	delete WorkingFrameInfo;
}

/** Sets the pending frame info */
void GRenderThread::SetPendingFrameInfo(FrameInfo* fi)
{
	MutexList.FrameCopyMutex.lock();

	delete PendingFrameInfo; // Delete it, to drop a frame in case the renderer didnt catch up
	PendingFrameInfo = fi;

	MutexList.FrameCopyMutex.unlock();
}

/** Sends the kill-signal to the thread */
void GRenderThread::KillThread()
{
	ThreadInfo.KillThread = true;
	RenderThread.join();
}

/** Starts the threads */
XRESULT GRenderThread::InitThreads()
{
	LogInfo() << "Starting rendering thread";

	// Start renderthread
	RenderThread = std::thread(GRenderThread::RenderThreadFunc, this);
}

/** Render thread func */
void GRenderThread::RenderThreadFunc(GRenderThread* obj)
{
	while(!obj->ThreadInfo.KillThread)
	{
		obj->DrawFrame();
	}
}

/** Draws a frame */
void GRenderThread::DrawFrame()
{
	BaseGraphicsEngine* engine = Engine::GraphicsEngine;

	// Check if we have a pending frame
	if(!PendingFrameInfo)
	{
		return;
	}

	// Set pending frame as current working copy and NULL it
	MutexList.FrameCopyMutex.lock();

	WorkingFrameInfo = PendingFrameInfo;
	PendingFrameInfo = NULL;

	MutexList.FrameCopyMutex.unlock();

	// Initial clear
	engine->Clear(float4(1,0,0,0));

	// Go through meshes
	for(std::list<std::pair<MaterialRenderInfo, std::list<MeshRenderInfo>>>::iterator itm = WorkingFrameInfo->Meshes.begin();
		itm != WorkingFrameInfo->Meshes.end(); itm++)
	{
		// Bind material
		if((*itm).first.Texture)(*itm).first.Texture->BindToPixelShader(0);
		if((*itm).first.NormalMap)(*itm).first.NormalMap->BindToPixelShader(1);

		for(std::list<MeshRenderInfo>::iterator itr = (*itm).second.begin(); itr != (*itm).second.end(); itr++)
		{
			(*itr).MeshConstantBuffer->BindToVertexShader(0);

			// Draw mesh-part
			Engine::GraphicsEngine->DrawVertexBufferIndexed((*itr).Mesh->MeshVertexBuffer, (*itr).Mesh->MeshIndexBuffer, (*itr).Mesh->Indices.size());			
		}
	}

	engine->Present();

	// Clean our working copy
	delete WorkingFrameInfo;
	WorkingFrameInfo = NULL;
}