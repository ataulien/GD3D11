#include "pch.h"
#include "D3D11PointLight.h"
#include "RenderToTextureBuffer.h"
#include "D3D11GraphicsEngineBase.h"
#include "D3D11GraphicsEngine.h" // TODO: Remove and use newer system!
#include "Engine.h"
#include "zCVobLight.h"
#include "BaseLineRenderer.h"
#include "WorldConverter.h"

const float LIGHT_COLORCHANGE_POS_MOD = 0.1f;

D3D11LightCreatorThread::D3D11LightCreatorThread()
{
	EndThread = false;
	IsRunning = false;
}

void D3D11LightCreatorThread::RunThread()
{
	IsRunning = true;
	Thread = new std::thread(Threadfunc, this);
}

D3D11LightCreatorThread::~D3D11LightCreatorThread()
{
	EndThread = true;
	CV.notify_all();

	Thread->join();
	delete Thread;
}

void D3D11LightCreatorThread::Threadfunc(D3D11LightCreatorThread* t)
{
	//std::unique_lock<std::mutex> lk(t->EmptyMutex);

	while(!t->EndThread)
	{
		while(t->Queue.empty())
			Sleep(10);
			//t->CV.wait(lk); // Wait when we don't have something to do

		if(t->EndThread)
			break; // check again, if EndThread was set to true while we were waiting

		// Get piece of work
		t->QueueMutex.lock();
		D3D11PointLight* p = t->Queue.back();
		t->Queue.pop_back();
		t->QueueMutex.unlock();

		// Init light
		p->InitResources();
	}
}

D3D11PointLight::D3D11PointLight(VobLightInfo* info, bool dynamicLight)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	LightInfo = info;
	DynamicLight = dynamicLight;

	LastUpdatePosition = LightInfo->Vob->GetPositionWorld();

	DepthCubemap = NULL;
	ViewMatricesCB = NULL;

	if(!dynamicLight)
	{
		InitDone = false;

		// Init thread when wanted
		if(!D3D11PointlightCreatorNS::CreatorThread.IsRunning)
		{
			D3D11PointlightCreatorNS::CreatorThread.RunThread();
		}

		// Add to queue
		D3D11PointlightCreatorNS::CreatorThread.QueueMutex.lock();
		D3D11PointlightCreatorNS::CreatorThread.Queue.push_back(this);
		D3D11PointlightCreatorNS::CreatorThread.QueueMutex.unlock();

		// Notify the thread
		D3D11PointlightCreatorNS::CreatorThread.CV.notify_all();
	}else
	{
		InitResources();
	}

	DrawnOnce = false;
}


D3D11PointLight::~D3D11PointLight(void)
{
	// Make sure we are out of the init-queue
	while(!InitDone);

	delete DepthCubemap;
	delete ViewMatricesCB;

	for(auto it=WorldMeshCache.begin();it!=WorldMeshCache.end();it++)
		delete (*it).second;
}

/** Returns true if this is the first time that light is being rendered */
bool D3D11PointLight::NotYetDrawn()
{
	return !DrawnOnce;
}

/** Initializes the resources of this light */
void D3D11PointLight::InitResources()
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;

	// Create texture-cube for this light
	DepthCubemap = new RenderToDepthStencilBuffer(engine->GetDevice(), 
		POINTLIGHT_SHADOWMAP_SIZE,
		POINTLIGHT_SHADOWMAP_SIZE,
		DXGI_FORMAT_R32_TYPELESS,
		NULL,
		DXGI_FORMAT_D32_FLOAT,
		DXGI_FORMAT_R32_FLOAT,
		6);

	// Create constantbuffer for the view-matrices
	engine->CreateConstantBuffer(&ViewMatricesCB, NULL, sizeof(CubemapGSConstantBuffer));

	// Generate worldmesh cache if we aren't a dynamically added light
	if(!DynamicLight)
	{
		WorldConverter::WorldMeshCollectPolyRange(LightInfo->Vob->GetPositionWorld(), LightInfo->Vob->GetLightRange(), Engine::GAPI->GetWorldSections(), WorldMeshCache);
		WorldCacheInvalid = false;
	}else
	{
		WorldCacheInvalid = true;
	}
	/*DebugTextureCubemap = new RenderToTextureBuffer(engine->GetDevice(), 
		POINTLIGHT_SHADOWMAP_SIZE,
		POINTLIGHT_SHADOWMAP_SIZE,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		NULL,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		1,
		6);*/

	// Init the cubemap
	//RenderCubemap(true);

	InitDone = true;
}

/** Returns if this light needs an update */
bool D3D11PointLight::NeedsUpdate()
{
	return LightInfo->Vob->GetPositionWorld() != LastUpdatePosition || NotYetDrawn();
}

/** Returns true if the light could need an update, but it's not very important */
bool D3D11PointLight::WantsUpdate()
{
	// If dynamic, update colorchanging lights too, because they are mostly lamps and campfires
	// They wouldn't need an update just because of the colorchange, but most of them are dominant lights so it looks better
	if(Engine::GAPI->GetRendererState()->RendererSettings.EnablePointlightShadows >= GothicRendererSettings::PLS_UPDATE_DYNAMIC)
		if(LightInfo->Vob->GetLightColor() != LastUpdateColor)
			return true;

	return false;
}

/** Draws the surrounding scene into the cubemap */
void D3D11PointLight::RenderCubemap(bool forceUpdate)
{
	if(!InitDone)
		return;

	//if(!GetAsyncKeyState('X'))
	//	return;
	D3D11GraphicsEngineBase* engineBase = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *) engineBase; // TODO: Remove and use newer system!


	D3DXVECTOR3 vEyePt = LightInfo->Vob->GetPositionWorld();
	//vEyePt += D3DXVECTOR3(0,1,0) * 20.0f; // Move lightsource out of the ground or other objects (torches!)
	// TODO: Move the actual lightsource up too!

	/*if(WantsUpdate())
	{
		// Move lights with colorchanges around a bit to make it look more light torches
		vEyePt.y += (float4(LastUpdateColor).x - 0.5f) * LIGHT_COLORCHANGE_POS_MOD;
		vEyePt.x += (float4(LastUpdateColor).y - 0.5f) * LIGHT_COLORCHANGE_POS_MOD;
		vEyePt.z += (float4(LastUpdateColor).y - 0.5f) * LIGHT_COLORCHANGE_POS_MOD;
	}*/

    D3DXVECTOR3 vLookDir;
    D3DXVECTOR3 vUpDir;

	if(!NeedsUpdate() && !WantsUpdate())
	{
		if(!forceUpdate)
			return; // Don't update when we don't need to
	}else
	{
		if(LightInfo->Vob->GetPositionWorld() != LastUpdatePosition)
		{
			// Position changed, refresh our caches
			VobCache.clear();
			SkeletalVobCache.clear();

			// Invalidate worldcache
			WorldCacheInvalid = true;
		}
	}

	

	// Update indoor/outdoor-state
	LightInfo->IsIndoorVob = LightInfo->Vob->IsIndoorVob();

	D3DXMATRIX proj;

	const bool dbg = false;

	// Generate cubemap view-matrices
    vLookDir = D3DXVECTOR3( 1.0f, 0.0f, 0.0f ) + vEyePt;
    vUpDir = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	if(dbg)Engine::GraphicsEngine->GetLineRenderer()->AddLine(LineVertex(vEyePt, float4(1.0f,0,0,1)), LineVertex((vLookDir - vEyePt) * 50.0f + vEyePt, float4(1.0f,1.0f,0,1)));
    D3DXMatrixLookAtLH( &CubeMapViewMatrices[0], &vEyePt, &vLookDir, &vUpDir );
    vLookDir = D3DXVECTOR3( -1.0f, 0.0f, 0.0f ) + vEyePt;
    vUpDir = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	if(dbg)Engine::GraphicsEngine->GetLineRenderer()->AddLine(LineVertex(vEyePt, float4(1.0f,0,0,1)), LineVertex((vLookDir - vEyePt) * 50.0f + vEyePt, float4(1.0f,1.0f,0,1)));
    D3DXMatrixLookAtLH( &CubeMapViewMatrices[1], &vEyePt, &vLookDir, &vUpDir );
    vLookDir = D3DXVECTOR3( 0.0f, 0.0f + 1.0f, 0.0f ) + vEyePt;
    vUpDir = D3DXVECTOR3( 0.0f, 0.0f, -1.0f );
	if(dbg)Engine::GraphicsEngine->GetLineRenderer()->AddLine(LineVertex(vEyePt, float4(1.0f,0,0,1)), LineVertex((vLookDir - vEyePt) * 50.0f + vEyePt, float4(1.0f,1.0f,0,1)));
    D3DXMatrixLookAtLH( &CubeMapViewMatrices[2], &vEyePt, &vLookDir, &vUpDir );
    vLookDir = D3DXVECTOR3( 0.0f, 0.0f - 1.0f, 0.0f ) + vEyePt;
    vUpDir = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
	if(dbg)Engine::GraphicsEngine->GetLineRenderer()->AddLine(LineVertex(vEyePt, float4(1.0f,0,0,1)), LineVertex((vLookDir - vEyePt) * 50.0f + vEyePt, float4(1.0f,1.0f,0,1)));
    D3DXMatrixLookAtLH( &CubeMapViewMatrices[3], &vEyePt, &vLookDir, &vUpDir );
    vLookDir = D3DXVECTOR3( 0.0f, 0.0f, 1.0f ) + vEyePt;
    vUpDir = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	if(dbg)Engine::GraphicsEngine->GetLineRenderer()->AddLine(LineVertex(vEyePt, float4(1.0f,0,0,1)), LineVertex((vLookDir - vEyePt) * 50.0f + vEyePt, float4(1.0f,1.0f,0,1)));
    D3DXMatrixLookAtLH( &CubeMapViewMatrices[4], &vEyePt, &vLookDir, &vUpDir );
    vLookDir = D3DXVECTOR3( 0.0f, 0.0f, -1.0f ) + vEyePt;
    vUpDir = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
	if(dbg)Engine::GraphicsEngine->GetLineRenderer()->AddLine(LineVertex(vEyePt, float4(1.0f,0,0,1)), LineVertex((vLookDir - vEyePt) * 50.0f + vEyePt, float4(1.0f,1.0f,0,1)));
    D3DXMatrixLookAtLH( &CubeMapViewMatrices[5], &vEyePt, &vLookDir, &vUpDir );

	for(int i=0;i<6;i++)
		D3DXMatrixTranspose(&CubeMapViewMatrices[i], &CubeMapViewMatrices[i]);

	// Create the projection matrix
	float zNear = 15.0f;
	float zFar = LightInfo->Vob->GetLightRange() * 2.0f;
    D3DXMatrixPerspectiveFovLH( &proj, D3DX_PI * 0.5f, 1.0f, zNear, zFar );
	D3DXMatrixTranspose(&proj, &proj);

	// Setup near/far-planes. We need linear viewspace depth for the cubic shadowmaps.
	Engine::GAPI->GetRendererState()->GraphicsState.FF_zNear = zNear;
	Engine::GAPI->GetRendererState()->GraphicsState.FF_zFar = zFar;
	Engine::GAPI->GetRendererState()->GraphicsState.SetGraphicsSwitch(GSWITCH_LINEAR_DEPTH, true);

	bool oldDepthClip = Engine::GAPI->GetRendererState()->RasterizerState.DepthClipEnable;
	Engine::GAPI->GetRendererState()->RasterizerState.DepthClipEnable = true;

	// Upload view-matrices to the GPU
	CubemapGSConstantBuffer gcb;
	for(int i=0;i<6;i++)
	{
		gcb.PCR_View[i] = CubeMapViewMatrices[i];
		gcb.PCR_ViewProj[i] = proj * CubeMapViewMatrices[i];
	}

	ViewMatricesCB->UpdateBuffer(&gcb);
	ViewMatricesCB->BindToGeometryShader(2);

	//for(int i=0;i<6;i++)
	//	RenderCubemapFace(CubeMapViewMatrices[i], proj, i);
	RenderFullCubemap();

	if(dbg)
	{
		for(auto it = VobCache.begin();it!=VobCache.end();it++)
			Engine::GraphicsEngine->GetLineRenderer()->AddLine(LineVertex(vEyePt, float4(1.0f,0,0,1)), LineVertex((*it)->Vob->GetPositionWorld(), float4(1.0f,1.0f,0,1)));
	}

	Engine::GAPI->GetRendererState()->RasterizerState.DepthClipEnable = oldDepthClip;
	Engine::GAPI->GetRendererState()->GraphicsState.SetGraphicsSwitch(GSWITCH_LINEAR_DEPTH, false);

	LastUpdateColor = LightInfo->Vob->GetLightColor();
	LastUpdatePosition = vEyePt;
	DrawnOnce = true;
}

/** Renders all cubemap faces at once, using the geometry shader */
void D3D11PointLight::RenderFullCubemap()
{
	D3D11GraphicsEngineBase* engineBase = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *) engineBase; // TODO: Remove and use newer system!

	// Disable shadows for NPCs
	// TODO: Only for the player himself, because his shadows look ugly when using a torch
	bool oldDrawSkel = Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes;
	//Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes = false;

	float range = LightInfo->Vob->GetLightRange() * 1.1f;

	// Draw no npcs if this is a static light. This is archived by simply not drawing them in the first update
	bool noNPCs = !DrawnOnce;//!LightInfo->Vob->IsStatic();

	// Draw cubemap
	std::map<MeshKey, WorldMeshInfo*, cmpMeshKey>* wc = &WorldMeshCache;
	if(&WorldCacheInvalid)
		wc = NULL;

	engine->RenderShadowCube(LightInfo->Vob->GetPositionWorld(), range, DepthCubemap, NULL, NULL, false, LightInfo->IsIndoorVob, noNPCs, &VobCache, &SkeletalVobCache, wc);

	//Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes = oldDrawSkel;
}

/** Renders the scene with the given view-proj-matrices */
void D3D11PointLight::RenderCubemapFace(const D3DXMATRIX& view, const D3DXMATRIX& proj, UINT faceIdx)
{
	D3D11GraphicsEngineBase* engineBase = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *) engineBase; // TODO: Remove and use newer system!
	CameraReplacement cr;
	cr.PositionReplacement = LightInfo->Vob->GetPositionWorld();
	cr.ProjectionReplacement = proj;
	cr.ViewReplacement = view;

	// Replace gothics camera
	Engine::GAPI->SetCameraReplacementPtr(&cr);

	if(engine->GetDummyCubeRT())
		engine->GetContext()->ClearRenderTargetView(engine->GetDummyCubeRT()->GetRTVCubemapFace(faceIdx), (float *)&float4(0,0,0,0));

	// Disable shadows for NPCs
	// TODO: Only for the player himself, because his shadows look ugly when using a torch
	bool oldDrawSkel = Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes;
	//Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes = false;

	float range = LightInfo->Vob->GetLightRange() * 1.1f;

	// Draw cubemap face
	ID3D11RenderTargetView* debugRTV = engine->GetDummyCubeRT() != NULL ? engine->GetDummyCubeRT()->GetRTVCubemapFace(faceIdx) : NULL;
	engine->RenderShadowCube(cr.PositionReplacement, range, DepthCubemap, DepthCubemap->GetDSVCubemapFace(faceIdx), debugRTV, false);

	//Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes = oldDrawSkel;

	// Reset settings
	Engine::GAPI->SetCameraReplacementPtr(NULL);
}

/** Binds the shadowmap to the pixelshader */
void D3D11PointLight::OnRenderLight()
{	
	if(!InitDone)
		return;

	DepthCubemap->BindToPixelShader(((D3D11GraphicsEngineBase *)Engine::GraphicsEngine)->GetContext(), 3);
}

/** Debug-draws the cubemap to the screen */
void D3D11PointLight::DebugDrawCubeMap()
{
	if(!InitDone)
		return;

	D3D11GraphicsEngineBase* engineBase = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *) engineBase; // TODO: Remove and use newer system!

	const int previewSize = POINTLIGHT_SHADOWMAP_SIZE;
	const int previewDownscale = 4;
	


	for(int i=0;i<6;i++)
	{
		INT2 pPosition;
		int stride = (previewSize / previewDownscale);
		if(i==1) // x-
		{
			pPosition.x = 0;
			pPosition.y = stride;
		}else if(i==3) // y-
		{
			pPosition.x = stride;
			pPosition.y = stride;
		}else if(i==0) // x+
		{
			pPosition.x = stride * 2;
			pPosition.y = stride;
		}else if(i==2) // y+
		{
			pPosition.x = stride * 3;
			pPosition.y = stride;
		}else if(i==5) // z-
		{
			pPosition.x = stride;
			pPosition.y = 0;
		}else if(i==4) // z+
		{
			pPosition.x = stride;
			pPosition.y = stride * 2;
		}

		INT2 pSize = INT2(previewSize / previewDownscale,previewSize / previewDownscale);

		ID3D11ShaderResourceView* srv = engine->GetDummyCubeRT()->GetSRVCubemapFace(i);
		engine->GetContext()->PSSetShaderResources(0,1, &srv);
		Engine::GraphicsEngine->DrawQuad(pPosition, pSize);
	}
}

/** Called when a vob got removed from the world */
void D3D11PointLight::OnVobRemovedFromWorld(BaseVobInfo* vob)
{
	// See if we have this vob registered
	if(std::find(VobCache.begin(), VobCache.end(), vob) != VobCache.end()
		|| std::find(SkeletalVobCache.begin(), SkeletalVobCache.end(), vob) != SkeletalVobCache.end())
	{
		// Clear cache, if so
		VobCache.clear();
		SkeletalVobCache.clear();
	}
}