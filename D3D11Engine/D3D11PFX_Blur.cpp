#include "pch.h"
#include "D3D11PFX_Blur.h"
#include "Engine.h"
#include "D3D11GraphicsEngine.h"
#include "D3D11PfxRenderer.h"
#include "RenderToTextureBuffer.h"
#include "D3D11ShaderManager.h"
#include "D3D11VShader.h"
#include "D3D11PShader.h"
#include "D3D11ConstantBuffer.h"
#include "ConstantBufferStructs.h"

D3D11PFX_Blur::D3D11PFX_Blur(D3D11PfxRenderer* rnd) : D3D11PFX_Effect(rnd)
{

}


D3D11PFX_Blur::~D3D11PFX_Blur(void)
{

}

/** Draws this effect to the given buffer */
XRESULT D3D11PFX_Blur::RenderBlur(RenderToTextureBuffer* fxbuffer, bool leaveResultInD4_2, float threshold, float scale, const D3DXVECTOR4& colorMod, const std::string& finalCopyShader)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	// Save old rendertargets
	ID3D11RenderTargetView* oldRTV = NULL;
	ID3D11DepthStencilView* oldDSV = NULL;
	engine->GetContext()->OMGetRenderTargets(1, &oldRTV, &oldDSV);

	INT2 dsRes = INT2(fxbuffer->GetSizeX() / 4, fxbuffer->GetSizeY() / 4);

	/** Pass 1: Downscale/Blur-H */
	// Apply PFX-VS
	engine->GetShaderManager()->GetVShader("VS_PFX")->Apply();
	D3D11PShader* gaussPS = engine->GetShaderManager()->GetPShader("PS_PFX_GaussBlur");
	D3D11PShader* simplePS = engine->GetShaderManager()->GetPShader(finalCopyShader);

	// Apply blur-H shader
	gaussPS->Apply();

	// Update settings
	BlurConstantBuffer bcb;
	bcb.B_BlurSize = scale;
	bcb.B_PixelSize = float2(1.0f / FxRenderer->GetTempBufferDS4_1()->GetSizeX(), 0.0f);
	bcb.B_Threshold = threshold;
	bcb.B_ColorMod = colorMod;
	gaussPS->GetConstantBuffer()[0]->UpdateBuffer(&bcb);
	gaussPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	// Bind depthbuffer
	//engine->GetDepthBuffer()->BindToPixelShader(engine->GetContext(), 1);

	// Copy
	FxRenderer->CopyTextureToRTV(fxbuffer->GetShaderResView(), FxRenderer->GetTempBufferDS4_1()->GetRenderTargetView(), dsRes, true);

	/** Pass 2: Blur V */

	// Update settings
	bcb.B_BlurSize = scale;
	bcb.B_PixelSize = float2(0.0f, 1.0f / FxRenderer->GetTempBufferDS4_1()->GetSizeY());
	bcb.B_Threshold = 0.0f;
	gaussPS->GetConstantBuffer()[0]->UpdateBuffer(&bcb);
	gaussPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	// Copy
	FxRenderer->CopyTextureToRTV(FxRenderer->GetTempBufferDS4_1()->GetShaderResView(), FxRenderer->GetTempBufferDS4_2()->GetRenderTargetView(), dsRes, true);

	/** Pass 3: Copy back to FX-Buffer */

	if(!leaveResultInD4_2)
	{
		simplePS->Apply();
		FxRenderer->CopyTextureToRTV(FxRenderer->GetTempBufferDS4_2()->GetShaderResView(), fxbuffer->GetRenderTargetView(), INT2(0,0), true);
	}

	engine->GetContext()->OMSetRenderTargets(1, &oldRTV, oldDSV);
	if(oldRTV)oldRTV->Release();
	if(oldDSV)oldDSV->Release();

	return XR_SUCCESS;
}

/** Draws this effect to the given buffer */
XRESULT D3D11PFX_Blur::Render(RenderToTextureBuffer* fxbuffer)
{
	return RenderBlur(fxbuffer);
}