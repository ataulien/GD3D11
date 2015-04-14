#include "pch.h"
#include "D3D11PFX_GodRays.h"
#include "Engine.h"
#include "D3D11GraphicsEngine.h"
#include "D3D11PfxRenderer.h"
#include "RenderToTextureBuffer.h"
#include "D3D11ShaderManager.h"
#include "D3D11VShader.h"
#include "D3D11PShader.h"
#include "D3D11ConstantBuffer.h"
#include "ConstantBufferStructs.h"
#include "GothicAPI.h"
#include "GSky.h"

D3D11PFX_GodRays::D3D11PFX_GodRays(D3D11PfxRenderer* rnd) : D3D11PFX_Effect(rnd)
{
}


D3D11PFX_GodRays::~D3D11PFX_GodRays(void)
{
}

/** Draws this effect to the given buffer */
XRESULT D3D11PFX_GodRays::Render(RenderToTextureBuffer* fxbuffer)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	ID3D11RenderTargetView* oldRTV=NULL;
	ID3D11DepthStencilView* oldDSV=NULL;

	engine->GetContext()->OMGetRenderTargets(1, &oldRTV, &oldDSV);

	D3D11VShader* vs = engine->GetShaderManager()->GetVShader("VS_PFX");
	D3D11PShader* maskPS = engine->GetShaderManager()->GetPShader("PS_PFX_GodRayMask");
	D3D11PShader* zoomPS = engine->GetShaderManager()->GetPShader("PS_PFX_GodRayZoom");
	
	maskPS->Apply();
	vs->Apply();

	// Draw downscaled mask
	engine->GetContext()->OMSetRenderTargets(1, FxRenderer->GetTempBufferDS4_1()->GetRenderTargetViewPtr(), NULL);

	engine->GetHDRBackBuffer()->BindToPixelShader(engine->GetContext(), 0);
	engine->GetGBuffer1()->BindToPixelShader(engine->GetContext(), 1);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = (float)FxRenderer->GetTempBufferDS4_1()->GetSizeX();
	vp.Height = (float)FxRenderer->GetTempBufferDS4_1()->GetSizeY();

	engine->GetContext()->RSSetViewports(1, &vp);

	FxRenderer->DrawFullScreenQuad();

	// Zoom
	zoomPS->Apply();

	D3DXVECTOR3 sunPosition = *Engine::GAPI->GetSky()->GetAtmosphereCB().AC_LightPos.toD3DXVECTOR3();
	sunPosition *= Engine::GAPI->GetSky()->GetAtmosphereCB().AC_OuterRadius;
	sunPosition += Engine::GAPI->GetCameraPosition(); // Maybe use cameraposition from sky?

	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetProjectionMatrix();

	D3DXMATRIX viewProj = proj * view;
	D3DXMatrixTranspose(&viewProj, &viewProj);
	D3DXMatrixTranspose(&view, &view);

	D3DXVECTOR3 sunViewPosition;
	D3DXVec3TransformCoord(&sunViewPosition, &sunPosition, &view); // This is for checking if the light is behind the camera
	D3DXVec3TransformCoord(&sunPosition, &sunPosition, &viewProj);

	GodRayZoomConstantBuffer gcb;
	gcb.GR_Weight = 1.0f;
	gcb.GR_Decay = Engine::GAPI->GetRendererState()->RendererSettings.GodRayDecay;
	gcb.GR_Weight = Engine::GAPI->GetRendererState()->RendererSettings.GodRayWeight;
	gcb.GR_Density = Engine::GAPI->GetRendererState()->RendererSettings.GodRayDensity;
	
	gcb.GR_Center.x = sunPosition.x/2.0f +0.5f;
	gcb.GR_Center.y = sunPosition.y/-2.0f +0.5f;

	gcb.GR_ColorMod = Engine::GAPI->GetRendererState()->RendererSettings.GodRayColorMod;

	if(abs(gcb.GR_Center.x - 0.5f) > 0.5f)
		gcb.GR_Weight *= std::max(0.0f, 1.0f - (abs(gcb.GR_Center.x - 0.5f) - 0.5f) / 0.5f);

	if(abs(gcb.GR_Center.y - 0.5f) > 0.5f)
		gcb.GR_Weight *= std::max(0.0f, 1.0f - (abs(gcb.GR_Center.y - 0.5f) - 0.5f) / 0.5f);

	if(sunViewPosition.z < 0.0f)
		gcb.GR_Weight = 0.0f;

	zoomPS->GetConstantBuffer()[0]->UpdateBuffer(&gcb);
	zoomPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	FxRenderer->CopyTextureToRTV(FxRenderer->GetTempBufferDS4_1()->GetShaderResView(), FxRenderer->GetTempBufferDS4_2()->GetRenderTargetView(), INT2(0,0), true);

	// Upscale and blend
	Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	FxRenderer->CopyTextureToRTV(FxRenderer->GetTempBufferDS4_2()->GetShaderResView(), oldRTV, INT2(engine->GetResolution().x, engine->GetResolution().y));

	vp.Width = (float)engine->GetResolution().x;
	vp.Height = (float)engine->GetResolution().y;

	engine->GetContext()->RSSetViewports(1, &vp);

	engine->GetContext()->OMSetRenderTargets(1, &oldRTV, oldDSV);
	if(oldRTV)oldRTV->Release();
	if(oldDSV)oldDSV->Release();
	return XR_SUCCESS;
}