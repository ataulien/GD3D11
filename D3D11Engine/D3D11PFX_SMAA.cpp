#include "pch.h"
#include "D3D11PFX_SMAA.h"
#include "Logger.h"
#include "Engine.h"
#include "RenderToTextureBuffer.h"
#include "D3D11GraphicsEngine.h"
#include <D3DX11.h>
#include "D3D11PfxRenderer.h"
#include "D3D11ShaderManager.h"
#include "D3D11VShader.h"
#include "GothicAPI.h"
#include "D3D11PShader.h"

D3D11PFX_SMAA::D3D11PFX_SMAA(D3D11PfxRenderer* rnd) : D3D11PFX_Effect(rnd)
{
	EdgesTex = NULL;
	BlendTex = NULL;
	SMAAShader = NULL;
	AreaTextureSRV = NULL;
	SearchTextureSRV = NULL;

	Init();
}


D3D11PFX_SMAA::~D3D11PFX_SMAA(void)
{
	delete EdgesTex;
	delete BlendTex;

	if(AreaTextureSRV)AreaTextureSRV->Release();
	if(SearchTextureSRV)SearchTextureSRV->Release();
	if(SMAAShader)SMAAShader->Release();
}

HRESULT D3DX11CreateEffectFromFile_RES(
  LPCTSTR pFileName,
  CONST D3D10_SHADER_MACRO *pDefines,
  LPCSTR pProfile,
  UINT HLSLFlags,
  UINT FXFlags,
  ID3D11Device *pDevice,
  void *pPump,
  ID3DX11Effect **ppEffect,
  HRESULT *pHResult
)
{

	ID3D10Blob* ShaderBuffer;
	ID3D10Blob* ErrorsBuffer;
	HRESULT hr=D3DX11CompileFromFile(pFileName, pDefines, NULL, NULL, pProfile, HLSLFlags, FXFlags, NULL, &ShaderBuffer,&ErrorsBuffer,pHResult);


	char* Errors;
	if(ErrorsBuffer)
	{
		Errors=(char *)ErrorsBuffer->GetBufferPointer();

		CHAR Msg[5024];

		strcpy_s((char *)&Msg,ErrorsBuffer->GetBufferSize(),Errors);

		if(ShaderBuffer)
		{
			LogError() << ErrorsBuffer->GetBufferPointer();
			return E_FAIL;
		}else
		{
			//MessageBoxA(NULL,(char *)ErrorsBuffer->GetBufferPointer(),"There are warnings in that fx file!",MB_OK | MB_ICONWARNING | MB_TOPMOST);
		}
		ErrorsBuffer->Release();
	}

	D3DX11CreateEffectFromMemory(ShaderBuffer->GetBufferPointer(), ShaderBuffer->GetBufferSize(), HLSLFlags, pDevice, ppEffect);


	ShaderBuffer->Release();
	return S_OK;
}

/** Creates needed resources */
bool D3D11PFX_SMAA::Init()
{
	HRESULT hr;

	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;

	LE(D3DX11CreateEffectFromFile_RES("System\\GD3D11\\Shaders\\SMAA.fx", NULL, "fx_5_0", D3D10_SHADER_ENABLE_STRICTNESS | D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, engine->GetDevice(), NULL,&SMAAShader, NULL));
	
	if(AreaTextureSRV)AreaTextureSRV->Release();
	if(SearchTextureSRV)SearchTextureSRV->Release();


	/*SMAAShader->AddCustomVariable("colorTex", CVT_SHADER_RES_VIEW, &ColorTexIdx);
	SMAAShader->AddCustomVariable("colorTexGamma", CVT_SHADER_RES_VIEW, &ColorTexGammaIdx);
	SMAAShader->AddCustomVariable("colorTexPrev", CVT_SHADER_RES_VIEW, &ColorTexPrevIdx);
	SMAAShader->AddCustomVariable("colorMSTex", CVT_SHADER_RES_VIEW, &ColorMSTexIdx);
	SMAAShader->AddCustomVariable("depthTex", CVT_SHADER_RES_VIEW, &DepthTexIdx);
	SMAAShader->AddCustomVariable("velocityTex", CVT_SHADER_RES_VIEW, &VelocityTexIdx);
	SMAAShader->AddCustomVariable("edgesTex", CVT_SHADER_RES_VIEW, &EdgesTexIdx);
	SMAAShader->AddCustomVariable("blendTex", CVT_SHADER_RES_VIEW, &BlendTexIdx);
	SMAAShader->AddCustomVariable("areaTex", CVT_SHADER_RES_VIEW, &areaTexIdx);
	SMAAShader->AddCustomVariable("searchTex", CVT_SHADER_RES_VIEW, &SearchTexIdx);*/

	// Load the textures
	D3DX11_IMAGE_LOAD_INFO img;
	img.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	img.MipLevels = 1;
	hr = D3DX11CreateShaderResourceViewFromFile(engine->GetDevice(), "system\\shaders\\SMAA_AreaTexDX10.dds", &img, NULL, &AreaTextureSRV, &hr);
	LE(hr);

	img.Format = DXGI_FORMAT_R8_UNORM;
	hr = D3DX11CreateShaderResourceViewFromFile(engine->GetDevice(), "system\\shaders\\SMAA_SearchTex.dds", &img, NULL, &SearchTextureSRV, &hr);
	LE(hr);

	SMAAShader->GetVariableByName("areaTex")->AsShaderResource()->SetResource(AreaTextureSRV);
	SMAAShader->GetVariableByName("searchTex")->AsShaderResource()->SetResource(SearchTextureSRV);

	LumaEdgeDetection = SMAAShader->GetTechniqueByName("LumaEdgeDetection");
	BlendingWeightCalculation = SMAAShader->GetTechniqueByName("BlendingWeightCalculation");
	NeighborhoodBlending = SMAAShader->GetTechniqueByName("NeighborhoodBlending");

	return true;
}

/** Renders the PostFX */
void D3D11PFX_SMAA::RenderPostFX(ID3D11ShaderResourceView* renderTargetSRV)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;
	engine->SetDefaultStates();
	engine->UpdateRenderStates();

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 0.0f;
	vp.Width = (float)FxRenderer->GetTempBuffer()->GetSizeX();
	vp.Height = (float)FxRenderer->GetTempBuffer()->GetSizeY();

	engine->GetShaderManager()->GetVShader("VS_PFX")->Apply(); // Apply vertexlayout for PP-Effects

	RenderToTextureBuffer* TempRTV = FxRenderer->GetTempBuffer();

	if(!EdgesTex)
	{
		OnResize(INT2(engine->GetResolution().x, engine->GetResolution().y));
	}

	engine->GetContext()->ClearRenderTargetView(EdgesTex->GetRenderTargetView(), D3DXVECTOR4(0,0,0,0));
	engine->GetContext()->ClearRenderTargetView(BlendTex->GetRenderTargetView(), D3DXVECTOR4(0,0,0,0));

	ID3D11RenderTargetView* RTV=NULL;
	ID3D11RenderTargetView* OldRTV=NULL;
	ID3D11DepthStencilView* OldDSV=NULL;
	ID3DX11EffectShaderResourceVariable* SRV=NULL;

	engine->GetContext()->OMGetRenderTargets(1, &OldRTV, &OldDSV);
	engine->GetContext()->ClearDepthStencilView(OldDSV, D3D11_CLEAR_STENCIL, 0, 0);

	/** First pass - Edge detection */
	RTV = EdgesTex->GetRenderTargetView();
	engine->GetContext()->OMSetRenderTargets(1, &RTV, OldDSV);
	
	SMAAShader->GetVariableByName("colorTexGamma")->AsShaderResource()->SetResource(renderTargetSRV);
	
	LumaEdgeDetection->GetPassByIndex(0)->Apply(0, engine->GetContext());
	FxRenderer->DrawFullScreenQuad();

	//FxRenderer->CopyTextureToRTV(renderTargetSRV, RTV, INT2(0,0), true);

	SMAAShader->GetVariableByName("colorTexGamma")->AsShaderResource()->SetResource(NULL);
	
	ID3D11ShaderResourceView *const NoSRV[3] = {NULL,NULL, NULL};
	engine->GetContext()->PSSetShaderResources(0, 3, NoSRV);

	/** Second pass - BlendingWeightCalculation */
	RTV = BlendTex->GetRenderTargetView();
	engine->GetContext()->OMSetRenderTargets(1, &RTV, OldDSV);

	SMAAShader->GetVariableByName("edgesTex")->AsShaderResource()->SetResource(EdgesTex->GetShaderResView());

	BlendingWeightCalculation->GetPassByIndex(0)->Apply(0, engine->GetContext());
	FxRenderer->DrawFullScreenQuad();

	/** Copy back to main RTV */
	/*DXUTGetD3D11DeviceContext()->OMSetRenderTargets(1, &OldRTV, NULL);
	CopyShader->SetBackBufferVar(BlendTex->GetShaderResView());
	CmplxScreenQuad.SetShader(CopyShader);
	CmplxScreenQuad.Render(6);

	goto end;*/



	/** Third pass - NeighborhoodBlending */
	RTV = TempRTV->GetRenderTargetView();
	engine->GetContext()->OMSetRenderTargets(1, &RTV, OldDSV);


	SMAAShader->GetVariableByName("colorTex")->AsShaderResource()->SetResource(renderTargetSRV);
	SMAAShader->GetVariableByName("blendTex")->AsShaderResource()->SetResource(BlendTex->GetShaderResView());

	NeighborhoodBlending->GetPassByIndex(0)->Apply(0, engine->GetContext());
	FxRenderer->DrawFullScreenQuad();

	SMAAShader->GetVariableByName("colorTex")->AsShaderResource()->SetResource(NULL);
	
	/** Copy back to main RTV */
	engine->GetContext()->OMSetRenderTargets(1, &OldRTV, NULL);
	/*engine->GetContext()->OMSetRenderTargets(1, &OldRTV, NULL);
	engine->DrawSRVToBackbuffer(TempRTV->GetShaderResView());
	goto end;*/

	ID3D11ShaderResourceView* srv = TempRTV->GetShaderResView();
	engine->GetContext()->PSSetShaderResources(0, 1, &srv);


	if(Engine::GAPI->GetRendererState()->RendererSettings.SharpenFactor > 0.0f)
	{	
		D3D11PShader* sharpenPS = engine->GetShaderManager()->GetPShader("PS_PFX_Sharpen");
		sharpenPS->Apply();

		GammaCorrectConstantBuffer gcb;
		gcb.G_Gamma = Engine::GAPI->GetGammaValue();
		gcb.G_Brightness = Engine::GAPI->GetBrightnessValue();
		gcb.G_TextureSize = engine->GetResolution();
		gcb.G_SharpenStrength = Engine::GAPI->GetRendererState()->RendererSettings.SharpenFactor;

		sharpenPS->GetConstantBuffer()[0]->UpdateBuffer(&gcb);
		sharpenPS->GetConstantBuffer()[0]->BindToPixelShader(0);

		FxRenderer->CopyTextureToRTV(TempRTV->GetShaderResView(), OldRTV, INT2(0,0), true);
	}else
	{
		FxRenderer->CopyTextureToRTV(TempRTV->GetShaderResView(), OldRTV);
	}

	engine->GetContext()->PSSetShaderResources(0, 3, NoSRV);

	engine->GetContext()->OMSetRenderTargets(1, &OldRTV, OldDSV);
	if(OldRTV)OldRTV->Release();
	if(OldDSV)OldDSV->Release();
}

/** Called on resize */
void D3D11PFX_SMAA::OnResize(const INT2& size)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;

	delete EdgesTex;
	delete BlendTex;

	HRESULT hr = S_OK;
	
	// Create Edges- and Blend-Textures
	EdgesTex = new RenderToTextureBuffer(engine->GetDevice(), size.x, size.y, DXGI_FORMAT_R8G8B8A8_UNORM, &hr);
	LE(hr);

	BlendTex = new RenderToTextureBuffer(engine->GetDevice(), size.x, size.y, DXGI_FORMAT_R8G8B8A8_UNORM, &hr);
	LE(hr);

	std::vector<D3D10_SHADER_MACRO> Makros;
	
	char ResStr[256];
	sprintf(ResStr, "float2(1.0f/%d, 1.0f/%d)", (int)(size.x), (int)(size.y));
	D3D10_SHADER_MACRO PixelSize = {"SMAA_PIXEL_SIZE", ResStr};
	Makros.push_back(PixelSize);

	const int QUALITY = 2;
	switch(QUALITY)
	{
	case 0:
		{
			D3D10_SHADER_MACRO Quality = {"SMAA_PRESET_LOW","1"};
			Makros.push_back(Quality);
		}
		break;

	case 1:
		{
			D3D10_SHADER_MACRO Quality = {"SMAA_PRESET_MEDIUM","1"};
			Makros.push_back(Quality);
		}
		break;

	case 2:
		{
			D3D10_SHADER_MACRO Quality = {"SMAA_PRESET_HIGH","1"};
			Makros.push_back(Quality);
		}
		break;

	case 3:
		{
			D3D10_SHADER_MACRO Quality = {"SMAA_PRESET_ULTRA","1"};
			Makros.push_back(Quality);
		}
		break;
	}
	D3D10_SHADER_MACRO Null = {NULL, NULL};
	Makros.push_back(Null);

	LE(D3DX11CreateEffectFromFile_RES("system\\GD3D11\\shaders\\SMAA.fx", &Makros[0], "fx_5_0", D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, engine->GetDevice(), NULL,&SMAAShader, NULL));
	
	if(AreaTextureSRV)AreaTextureSRV->Release();
	if(SearchTextureSRV)SearchTextureSRV->Release();

	// Load the textures
	D3DX11_IMAGE_LOAD_INFO img;
	img.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	img.MipLevels = 1;
	D3DX11CreateShaderResourceViewFromFile(engine->GetDevice(), "system\\GD3D11\\Textures\\SMAA_AreaTexDX10.dds", &img, NULL, &AreaTextureSRV, &hr);
	LE(hr);

	img.Format = DXGI_FORMAT_R8_UNORM;
	D3DX11CreateShaderResourceViewFromFile(engine->GetDevice(), "system\\GD3D11\\Textures\\SMAA_SearchTex.dds", &img, NULL, &SearchTextureSRV, &hr);
	LE(hr);

	SMAAShader->GetVariableByName("areaTex")->AsShaderResource()->SetResource(AreaTextureSRV);
	SMAAShader->GetVariableByName("searchTex")->AsShaderResource()->SetResource(SearchTextureSRV);

	LumaEdgeDetection = SMAAShader->GetTechniqueByName("LumaEdgeDetection");
	BlendingWeightCalculation = SMAAShader->GetTechniqueByName("BlendingWeightCalculation");
	NeighborhoodBlending = SMAAShader->GetTechniqueByName("NeighborhoodBlending");
}