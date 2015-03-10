#include "pch.h"
#include "D3D11AntTweakBar.h"
#include "AntTweakBar.h"
#include "Engine.h"
#include "D3D11GraphicsEngine.h"

D3D11AntTweakBar::D3D11AntTweakBar(void)
{
}


D3D11AntTweakBar::~D3D11AntTweakBar(void)
{
}

/** Creates the resources */
XRESULT D3D11AntTweakBar::Init()
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine *)Engine::GraphicsEngine;

	LogInfo() << "Initializing AntTweakBar";
	if(!TwInit(TW_DIRECT3D11, engine->GetDevice()))
		return XR_FAILED;

	TwWindowSize(engine->GetResolution().x, engine->GetResolution().y);

	return BaseAntTweakBar::Init();
}