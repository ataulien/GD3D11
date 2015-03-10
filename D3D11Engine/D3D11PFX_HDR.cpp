#include "pch.h"
#include "D3D11PFX_HDR.h"
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

const int LUM_SIZE = 512;

D3D11PFX_HDR::D3D11PFX_HDR(D3D11PfxRenderer* rnd) : D3D11PFX_Effect(rnd)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	// Create lum-buffer
	LumBuffer1 = new RenderToTextureBuffer(engine->GetDevice(), LUM_SIZE, LUM_SIZE, DXGI_FORMAT_R16_FLOAT, NULL, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, (int)(log(LUM_SIZE) / log(2)));
	LumBuffer2 = new RenderToTextureBuffer(engine->GetDevice(), LUM_SIZE, LUM_SIZE, DXGI_FORMAT_R16_FLOAT, NULL, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, (int)(log(LUM_SIZE) / log(2)));
	LumBuffer3 = new RenderToTextureBuffer(engine->GetDevice(), LUM_SIZE, LUM_SIZE, DXGI_FORMAT_R16_FLOAT, NULL, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN, (int)(log(LUM_SIZE) / log(2)));

	engine->GetContext()->ClearRenderTargetView(LumBuffer1->GetRenderTargetView(), (float *)&D3DXVECTOR4(0,0,0,0));
	engine->GetContext()->ClearRenderTargetView(LumBuffer2->GetRenderTargetView(), (float *)&D3DXVECTOR4(0,0,0,0));
	engine->GetContext()->ClearRenderTargetView(LumBuffer3->GetRenderTargetView(), (float *)&D3DXVECTOR4(0,0,0,0));
	ActiveLumBuffer = 0;
}


D3D11PFX_HDR::~D3D11PFX_HDR(void)
{
	delete LumBuffer1;
	delete LumBuffer2;
	delete LumBuffer3;
}

/** Draws this effect to the given buffer */
XRESULT D3D11PFX_HDR::Render(RenderToTextureBuffer* fxbuffer)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;
	
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = false;
	Engine::GAPI->GetRendererState()->BlendStateDirty = true;

	// Save old rendertargets
	ID3D11RenderTargetView* oldRTV = NULL;
	ID3D11DepthStencilView* oldDSV = NULL;
	engine->GetContext()->OMGetRenderTargets(1, &oldRTV, &oldDSV);


	RenderToTextureBuffer* lum = CalcLuminance();
	CreateBloom(lum);

	// Copy the original image to our temp-buffer
	FxRenderer->CopyTextureToRTV(engine->GetHDRBackBuffer()->GetShaderResView(), FxRenderer->GetTempBuffer()->GetRenderTargetView(), engine->GetResolution());

	// Bind scene and luminance
	FxRenderer->GetTempBuffer()->BindToPixelShader(engine->GetContext(), 0);
	lum->BindToPixelShader(engine->GetContext(), 1);

	// Bind bloom
	FxRenderer->GetTempBufferDS4_1()->BindToPixelShader(engine->GetContext(), 2);

	// Draw the HDR-Shader
	D3D11PShader* hps = engine->GetShaderManager()->GetPShader("PS_PFX_HDR");
	hps->Apply();

	HDRSettingsConstantBuffer hcb;
	hcb.HDR_LumWhite = Engine::GAPI->GetRendererState()->RendererSettings.HDRLumWhite;
	hcb.HDR_MiddleGray = Engine::GAPI->GetRendererState()->RendererSettings.HDRMiddleGray;
	hcb.HDR_Threshold = Engine::GAPI->GetRendererState()->RendererSettings.BloomThreshold;
	hcb.HDR_BloomStrength = Engine::GAPI->GetRendererState()->RendererSettings.BloomStrength;
	hps->GetConstantBuffer()[0]->UpdateBuffer(&hcb);
	hps->GetConstantBuffer()[0]->BindToPixelShader(0);

	FxRenderer->CopyTextureToRTV(FxRenderer->GetTempBuffer()->GetShaderResView(), oldRTV, engine->GetResolution(), true);

	// Show lumBuffer
	//FxRenderer->CopyTextureToRTV(currentLum->GetShaderResView(), oldRTV, INT2(LUM_SIZE,LUM_SIZE), false);

	// Restore rendertargets
	ID3D11ShaderResourceView* srv = NULL;
	engine->GetContext()->PSSetShaderResources(1,1,&srv);
	engine->GetContext()->OMSetRenderTargets(1, &oldRTV, oldDSV);
	if(oldRTV)oldRTV->Release();
	if(oldDSV)oldDSV->Release();

	return XR_SUCCESS;
}

/** Blurs the backbuffer and puts the result into TempBufferDS4_2*/
void D3D11PFX_HDR::CreateBloom(RenderToTextureBuffer* lum)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;
	
	INT2 dsRes = INT2(Engine::GraphicsEngine->GetResolution().x / 4, Engine::GraphicsEngine->GetResolution().y / 4);
	engine->GetShaderManager()->GetVShader("VS_PFX")->Apply();
	D3D11PShader* tonemapPS = engine->GetShaderManager()->GetPShader("PS_PFX_Tonemap");
	tonemapPS->Apply();

	HDRSettingsConstantBuffer hcb;
	hcb.HDR_LumWhite = Engine::GAPI->GetRendererState()->RendererSettings.HDRLumWhite;
	hcb.HDR_MiddleGray = Engine::GAPI->GetRendererState()->RendererSettings.HDRMiddleGray;
	hcb.HDR_Threshold = Engine::GAPI->GetRendererState()->RendererSettings.BloomThreshold;
	tonemapPS->GetConstantBuffer()[0]->UpdateBuffer(&hcb);
	tonemapPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	lum->BindToPixelShader(engine->GetContext(), 1);
	FxRenderer->CopyTextureToRTV(engine->GetHDRBackBuffer()->GetShaderResView(), FxRenderer->GetTempBufferDS4_1()->GetRenderTargetView(), dsRes, true);

	D3D11PShader* gaussPS = engine->GetShaderManager()->GetPShader("PS_PFX_GaussBlur");


	/** Pass 1: Blur-H */
	// Apply PFX-VS
	D3D11PShader* simplePS = engine->GetShaderManager()->GetPShader("PS_PFX_Simple");

	// Apply blur-H shader
	gaussPS->Apply();

	// Update settings 
	BlurConstantBuffer bcb;
	bcb.B_BlurSize = 1.0f;
	bcb.B_PixelSize = float2(1.0f / FxRenderer->GetTempBufferDS4_1()->GetSizeX(), 0.0f);
	gaussPS->GetConstantBuffer()[0]->UpdateBuffer(&bcb);
	gaussPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	// Copy
	FxRenderer->CopyTextureToRTV(FxRenderer->GetTempBufferDS4_1()->GetShaderResView(), FxRenderer->GetTempBufferDS4_2()->GetRenderTargetView(), dsRes, true);

	/** Pass 2: Blur V */

	// Update settings
	bcb.B_BlurSize = 1.0f;
	bcb.B_PixelSize = float2(0.0f, 1.0f / FxRenderer->GetTempBufferDS4_1()->GetSizeY());
	bcb.B_Threshold = 0.0f;
	gaussPS->GetConstantBuffer()[0]->UpdateBuffer(&bcb);
	gaussPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	// Copy
	FxRenderer->CopyTextureToRTV(FxRenderer->GetTempBufferDS4_2()->GetShaderResView(), FxRenderer->GetTempBufferDS4_1()->GetRenderTargetView(), dsRes, true);

}

/** Calcualtes the luminance */
RenderToTextureBuffer* D3D11PFX_HDR::CalcLuminance()
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	RenderToTextureBuffer* lumRTV;
	RenderToTextureBuffer* lastLum;
	RenderToTextureBuffer* currentLum;

	// Figure out which buffers to use where
	switch(ActiveLumBuffer)
	{
	case 0:
		lumRTV = LumBuffer1;
		lastLum = LumBuffer2;
		currentLum = LumBuffer3;
		break;

	case 1:
		lumRTV = LumBuffer3;
		lastLum = LumBuffer1;
		currentLum = LumBuffer2;
		break;

	case 2:
		lumRTV = LumBuffer2;
		lastLum = LumBuffer3;
		currentLum = LumBuffer1;
		break;
	}

	D3D11PShader* lps = engine->GetShaderManager()->GetPShader("PS_PFX_LumConvert");
	lps->Apply();

	// Convert the backbuffer to our luminance buffer
	FxRenderer->CopyTextureToRTV(engine->GetHDRBackBuffer()->GetShaderResView(), currentLum->GetRenderTargetView(), INT2(LUM_SIZE,LUM_SIZE), true);

	// Create the average luminance
	engine->GetContext()->GenerateMips(currentLum->GetShaderResView());

	D3D11PShader* aps = engine->GetShaderManager()->GetPShader("PS_PFX_LumAdapt");
	aps->Apply();

	LumAdaptConstantBuffer lcb;
	lcb.LC_DeltaTime = Engine::GAPI->GetDeltaTime();
	aps->GetConstantBuffer()[0]->UpdateBuffer(&lcb);
	aps->GetConstantBuffer()[0]->BindToPixelShader(0);

	// Bind luminances
	lastLum->BindToPixelShader(engine->GetContext(), 1);
	currentLum->BindToPixelShader(engine->GetContext(), 2);
	
	// Convert the backbuffer to our luminance buffer
	FxRenderer->CopyTextureToRTV(NULL, lumRTV->GetRenderTargetView(), INT2(LUM_SIZE,LUM_SIZE), true);

	// Create the average luminance
	engine->GetContext()->GenerateMips(lumRTV->GetShaderResView());

	// Increment
	ActiveLumBuffer++;
	ActiveLumBuffer = (ActiveLumBuffer == 3 ? 0 : ActiveLumBuffer);

	return lumRTV;
}