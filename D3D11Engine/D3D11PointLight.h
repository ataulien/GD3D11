#pragma once
#include "ShadowedPointLight.h"
#include "WorldConverter.h"
#include <thread>
#include <condition_variable>

class D3D11PointLight;
struct D3D11LightCreatorThread
{
	D3D11LightCreatorThread();
	~D3D11LightCreatorThread();

	void RunThread();

	static void Threadfunc(D3D11LightCreatorThread* t);

	std::thread* Thread;
	std::mutex QueueMutex;
	std::mutex EmptyMutex;
	std::condition_variable CV;
	std::vector<D3D11PointLight*> Queue;
	bool EndThread;
	bool IsRunning;
};

namespace D3D11PointlightCreatorNS
{
	__declspec(selectany) D3D11LightCreatorThread CreatorThread;
};

struct VobLightInfo;
struct RenderToDepthStencilBuffer;
struct RenderToTextureBuffer;
struct VobInfo;
struct SkeletalVobInfo;
class BaseConstantBuffer;
class D3D11PointLight : public ShadowedPointLight
{
public:
	D3D11PointLight(VobLightInfo* info, bool dynamicLight = false);
	~D3D11PointLight(void);

	/** Initializes the resources of this light */
	void InitResources();

	/** Draws the surrounding scene into the cubemap */
	void RenderCubemap(bool forceUpdate = false);

	/** Binds the shadowmap to the pixelshader */
	void OnRenderLight();

	/** Debug-draws the cubemap to the screen */
	void DebugDrawCubeMap();

	/** Returns if this light needs an update */
	bool NeedsUpdate();

	/** Returns true if the light could need an update, but it's not very important */
	bool WantsUpdate();

	/** Returns true if this is the first time that light is being rendered */
	bool NotYetDrawn();

	/** Called when a vob got removed from the world */
	virtual void OnVobRemovedFromWorld(BaseVobInfo* vob);

protected:
	/** Renders the scene with the given view-proj-matrices */
	void RenderCubemapFace(const D3DXMATRIX& view, const D3DXMATRIX& proj, UINT faceIdx);

	/** Renders all cubemap faces at once, using the geometry shader */
	void RenderFullCubemap();

	std::list<VobInfo*> VobCache;
	std::list<SkeletalVobInfo*> SkeletalVobCache;
	std::map<MeshKey, WorldMeshInfo*, cmpMeshKey> WorldMeshCache;
	bool WorldCacheInvalid;

	VobLightInfo* LightInfo;
	RenderToDepthStencilBuffer* DepthCubemap;
	D3DXMATRIX CubeMapViewMatrices[6];
	D3DXVECTOR3 LastUpdatePosition;
	DWORD LastUpdateColor;
	BaseConstantBuffer* ViewMatricesCB;
	bool DynamicLight;
	bool InitDone;
	bool DrawnOnce;

};

