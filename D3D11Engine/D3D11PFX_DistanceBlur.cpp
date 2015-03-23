#include "pch.h"
#include "D3D11PFX_DistanceBlur.h"
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

D3D11PFX_DistanceBlur::D3D11PFX_DistanceBlur(D3D11PfxRenderer* rnd) : D3D11PFX_Effect(rnd)
{
}


D3D11PFX_DistanceBlur::~D3D11PFX_DistanceBlur(void)
{
}

/** Draws this effect to the given buffer */
XRESULT D3D11PFX_DistanceBlur::Render(RenderToTextureBuffer* fxbuffer)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	// Save old rendertargets
	ID3D11RenderTargetView* oldRTV = NULL;
	ID3D11DepthStencilView* oldDSV = NULL;
	engine->GetContext()->OMGetRenderTargets(1, &oldRTV, &oldDSV);

	engine->GetShaderManager()->GetVShader("VS_PFX")->Apply();
	D3D11PShader* ps = engine->GetShaderManager()->GetPShader("PS_PFX_DistanceBlur");

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	// Copy scene
	engine->GetContext()->ClearRenderTargetView(FxRenderer->GetTempBuffer()->GetRenderTargetView(), (float *)&D3DXVECTOR4(0,0,0,0));
	FxRenderer->CopyTextureToRTV(engine->GetGBuffer0()->GetShaderResView(), FxRenderer->GetTempBuffer()->GetRenderTargetView(), Engine::GraphicsEngine->GetResolution());

	engine->GetContext()->OMSetRenderTargets(1, &oldRTV, NULL);

	// Bind textures
	FxRenderer->GetTempBuffer()->BindToPixelShader(engine->GetContext(), 0);
	engine->GetDepthBuffer()->BindToPixelShader(engine->GetContext(), 1);

	// Blur/Copy
	ps->Apply();

	FxRenderer->DrawFullScreenQuad();

	// Restore rendertargets
	engine->GetContext()->OMSetRenderTargets(1, &oldRTV, oldDSV);
	if(oldRTV)oldRTV->Release();
	if(oldDSV)oldDSV->Release();

	return XR_SUCCESS;
}