#pragma once
#include "pch.h"
#include <thread>

struct MeshInfo;
class BaseTexture;
class BaseConstantBuffer;
struct MeshRenderInfo
{
	MeshInfo* Mesh;
	BaseConstantBuffer* MeshConstantBuffer;
};

struct MaterialInfo;
struct MaterialRenderInfo
{
	MaterialInfo* MatInfo;
	BaseTexture* Texture;
	BaseTexture* NormalMap;
};

struct VobInfo;
struct FrameInfo
{
	std::list<std::pair<MaterialRenderInfo, std::list<MeshRenderInfo>>> Meshes;
	std::list<std::pair<MaterialRenderInfo, std::list<std::vector<ExVertexStruct>>>> UIVertices;
	D3DXMATRIX ViewMatrix;
	D3DXMATRIX ProjMatrix;
};

class GRenderThread
{
public:
	GRenderThread(void);
	~GRenderThread(void);

	/** Starts the threads */
	XRESULT InitThreads();

	/** Sets the pending frame info */
	void SetPendingFrameInfo(FrameInfo* fi);

	/** Sends the kill-signal to the thread */
	void KillThread();
protected:

	/** Render thread func */
	static void RenderThreadFunc(GRenderThread* obj);

	/** Draws a frame */
	void DrawFrame();

	/** Main render thread */
	std::thread RenderThread;

	/** Pending frame info */
	FrameInfo* PendingFrameInfo;

	/** Current working copy */
	FrameInfo* WorkingFrameInfo;

	/** Mutexes */
	struct MutexListStruct
	{
		std::mutex FrameCopyMutex;
	};
	MutexListStruct MutexList;

	/** Thread info */
	struct ThreadInfoStruct
	{
		ThreadInfoStruct()
		{
			KillThread = false;
		}

		bool KillThread;
	};
	ThreadInfoStruct ThreadInfo;
};

