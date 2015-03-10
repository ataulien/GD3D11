#include "pch.h"
#include "D3D11NVHBAO.h"
#include "Engine.h"
#include "D3D11GraphicsEngine.h"
#include "GFSDK_SSAO.h"
#include "RenderToTextureBuffer.h"
#include "GothicAPI.h"

#pragma comment(lib, "GFSDK_SSAO.win32.lib")

D3D11NVHBAO::D3D11NVHBAO(void)
{
}


D3D11NVHBAO::~D3D11NVHBAO(void)
{
}

/** Initializes the library */
XRESULT D3D11NVHBAO::Init()
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;

	GFSDK_SSAO_CustomHeap CustomHeap;
	CustomHeap.new_ = ::operator new;
	CustomHeap.delete_ = ::operator delete;

	GFSDK_SSAO_Status status;
	
	status = GFSDK_SSAO_CreateContext_D3D11(engine->GetDevice(), &AOContext, &CustomHeap);
	if(status != GFSDK_SSAO_OK)
	{
		LogError() << "Failed to initialize Nvidia HBAO+!";
		return XR_FAILED;
	}


	return XR_SUCCESS;
}

/** Renders the HBAO-Effect onto the given RTV */
XRESULT D3D11NVHBAO::Render(ID3D11RenderTargetView* rtv)
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;

	D3D11_VIEWPORT vp;
	UINT num = 1;
	engine->GetContext()->RSGetViewports(&num, &vp);

	HBAOSettings& settings = Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings;

	GFSDK_SSAO_InputData_D3D11 Input;
	Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
	Input.DepthData.pFullResDepthTextureSRV = engine->GetDepthBuffer()->GetShaderResView();
	Input.DepthData.pViewport = &vp;
	Input.DepthData.pProjectionMatrix = Engine::GAPI->GetProjectionMatrix();
	Input.DepthData.ProjectionMatrixLayout = GFSDK_SSAO_COLUMN_MAJOR_ORDER;
	Input.DepthData.MetersToViewSpaceUnits = settings.MetersToViewSpaceUnits;

	GFSDK_SSAO_Parameters_D3D11 Params;
	Params.Radius = settings.Radius;
	Params.Bias = settings.Bias;
	Params.PowerExponent = settings.PowerExponent;
	Params.Blur.Enable = true;
	Params.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_8;
	Params.Blur.Sharpness = settings.BlurSharpness;
	Params.Output.BlendMode = (GFSDK_SSAO_BlendMode)settings.BlendMode;

	GFSDK_SSAO_Status status;
	status = AOContext->RenderAO(engine->GetContext(), &Input, &Params, rtv);
	if(status != GFSDK_SSAO_OK)
	{
		LogError() << "Failed to render Nvidia HBAO+!";
		return XR_FAILED;
	}

	return XR_SUCCESS;
}