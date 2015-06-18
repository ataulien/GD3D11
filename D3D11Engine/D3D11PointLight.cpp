#include "pch.h"
#include "D3D11PointLight.h"
#include "RenderToTextureBuffer.h"
#include "D3D11GraphicsEngineBase.h"
#include "D3D11GraphicsEngine.h" // TODO: Remove and use newer system!
#include "Engine.h"
#include "zCVobLight.h"
#include "BaseLineRenderer.h"

D3D11PointLight::D3D11PointLight(VobLightInfo* info)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	LightInfo = info;

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


	/*DebugTextureCubemap = new RenderToTextureBuffer(engine->GetDevice(), 
		POINTLIGHT_SHADOWMAP_SIZE,
		POINTLIGHT_SHADOWMAP_SIZE,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		NULL,
		DXGI_FORMAT_UNKNOWN,
		DXGI_FORMAT_UNKNOWN,
		1,
		6);*/
}


D3D11PointLight::~D3D11PointLight(void)
{
	delete DepthCubemap;
	delete ViewMatricesCB;
}

/** Returns if this light needs an update */
bool D3D11PointLight::NeedsUpdate()
{
	return LightInfo->Vob->GetPositionWorld() != LastUpdatePosition;
}

/** Draws the surrounding scene into the cubemap */
void D3D11PointLight::RenderCubemap(bool forceUpdate)
{
	//if(!GetAsyncKeyState('X'))
	//	return;
	D3D11GraphicsEngineBase* engineBase = (D3D11GraphicsEngineBase *)Engine::GraphicsEngine;
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *) engineBase; // TODO: Remove and use newer system!


	D3DXVECTOR3 vEyePt = LightInfo->Vob->GetPositionWorld();
	//vEyePt += D3DXVECTOR3(0,1,0) * 20.0f; // Move lightsource out of the ground or other objects (torches!)
	// TODO: Move the actual lightsource up too!

    D3DXVECTOR3 vLookDir;
    D3DXVECTOR3 vUpDir;

	if(!NeedsUpdate())
	{
		if(!forceUpdate)
			return; // Don't update when we don't need to
	}else
	{
		// Position changed, refresh our caches
		VobCache.clear();
		SkeletalVobCache.clear();
	}

	LastUpdatePosition = vEyePt;

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

	// Draw cubemap
	engine->RenderShadowCube(LightInfo->Vob->GetPositionWorld(), range, DepthCubemap, NULL, NULL, false, LightInfo->IsIndoorVob, &VobCache, &SkeletalVobCache);

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
	DepthCubemap->BindToPixelShader(((D3D11GraphicsEngineBase *)Engine::GraphicsEngine)->GetContext(), 3);
}

/** Debug-draws the cubemap to the screen */
void D3D11PointLight::DebugDrawCubeMap()
{
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