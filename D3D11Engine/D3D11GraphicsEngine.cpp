#include "pch.h"
#include "D3D11GraphicsEngine.h"
#include <D3DX11.h>
#include "D3D11VertexBuffer.h"
#include "D3D11ShaderManager.h"
#include "D3D11PShader.h"
#include "D3D11VShader.h"
#include "D3D11HDShader.h"
#include "D3D11ConstantBuffer.h"
#include "D3D11Texture.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "Logger.h"
#include "RenderToTextureBuffer.h"
#include "GSky.h"
#include "GMesh.h"
#include "D3D11PfxRenderer.h"
#include "ConstantBufferStructs.h"
#include "BaseAntTweakBar.h"
#include "zCVob.h"
#include "zCVobLight.h"
#include "zCMaterial.h"
#include "zCLightmap.h"
#include "zCBspTree.h"
#include "D3D11LineRenderer.h"
#include "D3D7\MyDirectDrawSurface7.h"
#include "zCView.h"
#include "D2DView.h"
#include "GOcean.h"
#include "D2DDialog.h"
#include "AlignedAllocator.h"
#include "zCDecal.h"
#include "zCQuadMark.h"
#include "D2DEditorView.h"
#include <algorithm>
#include "D3D11PipelineStates.h"
#include "zCParticleFX.h"
#include "win32ClipboardWrapper.h"

//#include "MemoryTracker.h"

#pragma comment( lib, "dxguid.lib")

const int RES_UPSCALE = 1;
const INT2 DEFAULT_RESOLUTION = INT2(1920 * RES_UPSCALE, 1080*  RES_UPSCALE);
//const int WORLD_SHADOWMAP_SIZE = 1024;

const int NUM_UNLOADEDTEXCOUNT_FORCE_LOAD_TEXTURES = 100;

const float DEFAULT_FAR_PLANE = 50000.0f;
const D3DXVECTOR4 UNDERWATER_COLOR_MOD = D3DXVECTOR4(0.5f, 0.7f, 1.0f, 1.0f);

//#define DEBUG_D3D11

D3D11GraphicsEngine::D3D11GraphicsEngine(void)
{
	Resolution = DEFAULT_RESOLUTION;

	SwapChain = NULL;
	DXGIFactory = NULL;
	DXGIAdapter = NULL;
	OutputWindow = NULL;
	BackbufferRTV = NULL;
	DepthStencilBuffer = NULL;
	Device = NULL;
	Context = NULL;
	DistortionTexture = NULL;
	DeferredContext = NULL;
	UIView = NULL;
	DepthStencilBufferCopy = NULL;

	InfiniteRangeConstantBuffer = NULL;
	OutdoorSmallVobsConstantBuffer = NULL;
	OutdoorVobsConstantBuffer = NULL;
	ShadowmapSamplerState = NULL;
	DefaultSamplerState = NULL;
	HUDRasterizerState = NULL;
	WorldRasterizerState = NULL;
	DefaultDepthStencilState = NULL;

	FFRasterizerState = NULL;
	FFBlendState = NULL;
	FFDepthStencilState = NULL;

	ActiveHDS = NULL;
	ShaderManager = NULL;
	DynamicInstancingBuffer = NULL;
	TempVertexBuffer = NULL;
	NoiseTexture = NULL;

	ActivePS = NULL;
	PfxRenderer = NULL;
	CloudBuffer = NULL;
	BackbufferSRV = NULL;

	InverseUnitSphereMesh = NULL;
	ReflectionCube = NULL;

	GBuffer1_Normals_SpecIntens_SpecPower = NULL;
	GBuffer0_Diffuse = NULL;
	WorldShadowmap1 = NULL;
	HDRBackBuffer = NULL;
	CubeSamplerState = NULL;
	ClampSamplerState = NULL;

	RenderingStage = DES_MAIN;

	PresentPending = false;

	LineRenderer = new D3D11LineRenderer;


	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);

	// Match the resolution with the current desktop resolution
	Resolution = Engine::GAPI->GetRendererState()->RendererSettings.LoadedResolution;
}

D3D11GraphicsEngine::~D3D11GraphicsEngine(void)
{
	GothicDepthBufferStateInfo::DeleteCachedObjects();
	GothicBlendStateInfo::DeleteCachedObjects();
	GothicRasterizerStateInfo::DeleteCachedObjects();

	delete InfiniteRangeConstantBuffer;
	delete OutdoorSmallVobsConstantBuffer;
	delete OutdoorVobsConstantBuffer;

	delete QuadVertexBuffer;
	delete QuadIndexBuffer;

	delete LineRenderer;
	delete DepthStencilBuffer;
	delete DynamicInstancingBuffer;
	delete PfxRenderer;
	delete CloudBuffer;
	delete DistortionTexture;
	delete NoiseTexture;
	delete ShaderManager;
	delete TempVertexBuffer;
	delete GBuffer1_Normals_SpecIntens_SpecPower;
	delete GBuffer0_Diffuse;
	delete HDRBackBuffer;
	delete WorldShadowmap1;
	delete InverseUnitSphereMesh;
	delete UIView;

	if(ReflectionCube)ReflectionCube->Release();
	if (CubeSamplerState)CubeSamplerState->Release();
	if (ClampSamplerState)ClampSamplerState->Release();
	if (ShadowmapSamplerState)ShadowmapSamplerState->Release();
	if (FFRasterizerState)FFRasterizerState->Release();
	if (FFBlendState)FFBlendState->Release();
	if (FFDepthStencilState)FFDepthStencilState->Release();
	if (BackbufferSRV)BackbufferSRV->Release();
	if (DefaultSamplerState)DefaultSamplerState->Release();
	if (HUDRasterizerState)HUDRasterizerState->Release();
	if (WorldRasterizerState)WorldRasterizerState->Release();
	if (DefaultDepthStencilState)DefaultDepthStencilState->Release();
	
	if (BackbufferRTV)BackbufferRTV->Release();
	if (SwapChain)SwapChain->Release();
	
	if (Context)Context->Release();
	//if (DeferredContext)DeferredContext->Release();

	ID3D11Debug* d3dDebug;
	Device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));

	if(d3dDebug)
	{
		d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		d3dDebug->Release();
	}

	if(Device)Device->Release();
	
	if (DXGIAdapter)DXGIAdapter->Release();
	if (DXGIFactory)DXGIFactory->Release();

	//MemTrackerFinalReport();
}

/** Called when the game created it's window */
XRESULT D3D11GraphicsEngine::Init()
{
	HRESULT hr;

	LogInfo() << "Initializing Device...";

	// Create DXGI factory
	LE(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&DXGIFactory));
	LE(DXGIFactory->EnumAdapters(0, &DXGIAdapter)); // Get first adapter

	// Find out what we are rendering on to write it into the logfile
	DXGI_ADAPTER_DESC adpDesc;
	DXGIAdapter->GetDesc(&adpDesc);

	std::wstring wDeviceDescription(adpDesc.Description);
	std::string deviceDescription(wDeviceDescription.begin(), wDeviceDescription.end());
	LogInfo() << "Rendering on: " << deviceDescription.c_str();

	int flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	D3D_FEATURE_LEVEL featurelevel = D3D_FEATURE_LEVEL_11_0;

	// Create D3D11-Device
#ifndef DEBUG_D3D11
	LE(D3D11CreateDevice(DXGIAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags, &featurelevel, 1, D3D11_SDK_VERSION, &Device, NULL, &Context));
#else
	LE(D3D11CreateDevice(DXGIAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, flags | D3D11_CREATE_DEVICE_DEBUG, &featurelevel, 1, D3D11_SDK_VERSION, &Device, NULL, &Context));
#endif
	
	if(hr == DXGI_ERROR_UNSUPPORTED)
	{
		LogErrorBox() <<	"Your GPU (" << deviceDescription.c_str() << ") does not support Direct3D 11, so it can't run GD3D11!\n"
			"It has to be at least Featurelevel 11_0 compatible, which requires at least:"
			" *	Nvidia GeForce GTX4xx or higher"
			" *	AMD Radeon 5xxx or higher\n\n"
			"The game will now close.";
		exit(0);
	}
	
	LE(Device->CreateDeferredContext(0, &DeferredContext)); // Used for multithreaded texture loading
	
	LogInfo() << "Creating ShaderManager";

	ShaderManager = new D3D11ShaderManager();
	ShaderManager->Init();
	ShaderManager->LoadShaders();

	PS_DiffuseNormalmapped = ShaderManager->GetPShader("PS_DiffuseNormalmapped");
	PS_Diffuse = ShaderManager->GetPShader("PS_Diffuse");
	PS_DiffuseNormalmappedAlphatest = ShaderManager->GetPShader("PS_DiffuseNormalmappedAlphaTest");
	PS_DiffuseAlphatest = ShaderManager->GetPShader("PS_DiffuseAlphaTest");


	TempVertexBuffer = new D3D11VertexBuffer();
	TempVertexBuffer->Init(NULL, DRAWVERTEXARRAY_BUFFER_SIZE, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);

	DynamicInstancingBuffer = new D3D11VertexBuffer();
	DynamicInstancingBuffer->Init(NULL, INSTANCING_BUFFER_SIZE, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);


	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -3.402823466e+38F; // -FLT_MAX
	samplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MAX

	LE(Device->CreateSamplerState(&samplerDesc, &DefaultSamplerState));
	Context->PSSetSamplers(0, 1, &DefaultSamplerState);
	Context->VSSetSamplers(0, 1, &DefaultSamplerState);
	Context->DSSetSamplers(0, 1, &DefaultSamplerState);
	Context->HSSetSamplers(0, 1, &DefaultSamplerState);

	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	Device->CreateSamplerState(&samplerDesc, &ShadowmapSamplerState);
	Context->PSSetSamplers(2, 1, &ShadowmapSamplerState);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	Device->CreateSamplerState(&samplerDesc, &ClampSamplerState);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	Device->CreateSamplerState(&samplerDesc, &CubeSamplerState);

	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = true; // Gothics world vertices are CCW. That get's set by the GAPI-Graphics-State as well but is hardcoded here
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = false;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = true;

	LE(Device->CreateRasterizerState(&rasterizerDesc, &WorldRasterizerState));
	Context->RSSetState(WorldRasterizerState);

	rasterizerDesc.FrontCounterClockwise = false;
	LE(Device->CreateRasterizerState(&rasterizerDesc, &HUDRasterizerState));

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	// Depth test parameters
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	LE(Device->CreateDepthStencilState(&depthStencilDesc, &DefaultDepthStencilState));

	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_Ex");

	CreateTexture(&DistortionTexture);
	DistortionTexture->Init("system\\GD3D11\\textures\\distortion2.dds");

	CreateTexture(&NoiseTexture);
	NoiseTexture->Init("system\\GD3D11\\textures\\noise.png");

	InverseUnitSphereMesh = new GMesh;
	InverseUnitSphereMesh->LoadMesh("system\\GD3D11\\meshes\\icoSphere.obj");

	// Create distance-buffers
	CreateConstantBuffer((BaseConstantBuffer **)&InfiniteRangeConstantBuffer, NULL, sizeof(float4));
	CreateConstantBuffer((BaseConstantBuffer **)&OutdoorSmallVobsConstantBuffer, NULL, sizeof(float4));
	CreateConstantBuffer((BaseConstantBuffer **)&OutdoorVobsConstantBuffer, NULL, sizeof(float4));

	// Init inf-buffer now
	InfiniteRangeConstantBuffer->UpdateBuffer(&D3DXVECTOR4(FLT_MAX, 0, 0, 0));

	// Load reflectioncube
	if(S_OK != D3DX11CreateShaderResourceViewFromFile(Device, "system\\GD3D11\\Textures\\reflect_cube.dds", NULL, NULL, &ReflectionCube, NULL))
		LogWarn() << "Failed to load file: system\\GD3D11\\Textures\\reflect_cube.dds";


	// Init quad buffers
	Engine::GraphicsEngine->CreateVertexBuffer(&QuadVertexBuffer);
	QuadVertexBuffer->Init(NULL, 6 * sizeof(ExVertexStruct), BaseVertexBuffer::EBindFlags::B_VERTEXBUFFER, BaseVertexBuffer::EUsageFlags::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);


	Engine::GraphicsEngine->CreateVertexBuffer(&QuadIndexBuffer);
	QuadIndexBuffer->Init(NULL, 6 * sizeof(VERTEX_INDEX), BaseVertexBuffer::EBindFlags::B_INDEXBUFFER, BaseVertexBuffer::EUsageFlags::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	
	ExVertexStruct vx[6];
	ZeroMemory(vx, sizeof(vx));

	float scale = 1.0f;
	vx[0].Position = float3(-scale * 0.5f, -scale * 0.5f, 0.0f);
	vx[1].Position = float3(scale * 0.5f, -scale * 0.5f, 0.0f);
	vx[2].Position = float3(-scale * 0.5f, scale * 0.5f, 0.0f);

	vx[0].TexCoord = float2(0,0);
	vx[1].TexCoord = float2(1,0);
	vx[2].TexCoord = float2(0,1);

	vx[0].Color = 0xFFFFFFFF;
	vx[1].Color = 0xFFFFFFFF;
	vx[2].Color = 0xFFFFFFFF;

	vx[3].Position = float3(scale * 0.5f, -scale * 0.5f, 0.0f);
	vx[4].Position = float3(scale * 0.5f, scale * 0.5f, 0.0f);
	vx[5].Position = float3(-scale * 0.5f, scale * 0.5f, 0.0f);

	vx[3].TexCoord = float2(1,0);
	vx[4].TexCoord = float2(1,1);
	vx[5].TexCoord = float2(0,1);

	vx[3].Color = 0xFFFFFFFF;
	vx[4].Color = 0xFFFFFFFF;
	vx[5].Color = 0xFFFFFFFF;

	QuadVertexBuffer->UpdateBuffer(vx);

	VERTEX_INDEX indices[] = {0,1,2,3,4,5};
	QuadIndexBuffer->UpdateBuffer(indices, sizeof(indices));


	return XR_SUCCESS;
}

/** Called when the game created its window */
XRESULT D3D11GraphicsEngine::SetWindow(HWND hWnd)
{
	LogInfo() << "Creating swapchain";
	OutputWindow = hWnd;

	OnResize(Resolution);

	return XR_SUCCESS;
}

/** Called on window resize/resolution change */
XRESULT D3D11GraphicsEngine::OnResize(INT2 newSize)
{
	HRESULT hr;

	if(memcmp(&Resolution, &newSize, sizeof(newSize)) == 0 && SwapChain)
		return XR_SUCCESS; // Don't resize if we don't have to

	Resolution = newSize;
	INT2 bbres = GetBackbufferResolution();

	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	SetWindowPos(OutputWindow, NULL, 0, 0, desktopRect.right, desktopRect.bottom, 0);

	// Release all referenced buffer resources before we can resize the swapchain
	if (BackbufferRTV)BackbufferRTV->Release();
	if (BackbufferSRV)BackbufferSRV->Release();
	delete DepthStencilBuffer;
	DepthStencilBuffer = NULL;

	if(UIView)UIView->PrepareResize();

	if (!SwapChain)
	{
		LogInfo() << "Creating new swapchain! (Format: DXGI_FORMAT_R8G8B8A8_UNORM)";

		DXGI_SWAP_CHAIN_DESC scd;
		ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scd.BufferCount = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		scd.OutputWindow = OutputWindow;
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferDesc.Height = bbres.y;
		scd.BufferDesc.Width = bbres.x;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		bool windowed = Engine::GAPI->HasCommandlineParameter("ZWINDOW") ||
						Engine::GAPI->GetIntParamFromConfig("zStartupWindowed");
		scd.Windowed = windowed;

#ifdef BUILD_GOTHIC_1_08k
#ifdef PUBLIC_RELEASE
		scd.Windowed = false;
#else
		scd.Windowed = true;
#endif
#endif


		LE(DXGIFactory->CreateSwapChain(Device, &scd, &SwapChain));
		 
		if (!SwapChain)
		{
			LogError() << "Failed to create Swapchain! Program will now exit!";
			exit(0);
		}

		// Need to init AntTweakBar now that we have a working swapchain
		XLE(Engine::AntTweakBar->Init());
	}
	else
	{
		LogInfo() << "Resizing swapchain  (Format: DXGI_FORMAT_R8G8B8A8_UNORM)";

		if (FAILED(SwapChain->ResizeBuffers(1, bbres.x, bbres.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
		{
			LogError() << "Failed to resize swapchain!";
			return XR_FAILED;
		}
	}

	// Successfully resized swapchain, re-get buffers
	ID3D11Texture2D* backbuffer = NULL;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backbuffer);

	// Recreate RenderTargetView
	LE(Device->CreateRenderTargetView(backbuffer, nullptr, &BackbufferRTV));
	LE(Device->CreateShaderResourceView(backbuffer, nullptr, &BackbufferSRV));

	// Create main view
	if(!UIView)
	{
		if(Engine::GAPI->GetRendererState()->RendererSettings.EnableEditorPanel)
		{
			UIView = new D2DView;
			if(XR_SUCCESS != UIView->Init(bbres, backbuffer))
			{
				delete UIView;
				UIView = NULL;
			}
		}
	}
	else
	{
		UIView->Resize(Resolution, backbuffer);
	}

	backbuffer->Release();

	// Recreate DepthStencilBuffer
	delete DepthStencilBuffer;
	DepthStencilBuffer = new RenderToDepthStencilBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);

	delete DepthStencilBufferCopy;
	DepthStencilBufferCopy = new RenderToTextureBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_FLOAT);

	// Bind our newly created resources
	Context->OMSetRenderTargets(1, &BackbufferRTV, DepthStencilBuffer->GetDepthStencilView());

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)bbres.x;
	viewport.Height = (float)bbres.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	Context->RSSetViewports(1, &viewport);

	
	// Create PFX-Renderer
	if(!PfxRenderer)
		PfxRenderer = new D3D11PfxRenderer;

	PfxRenderer->OnResize(Resolution);

	delete CloudBuffer;
	CloudBuffer = new RenderToTextureBuffer(Device, bbres.x, bbres.y, DXGI_FORMAT_R8G8B8A8_UNORM);

	delete GBuffer1_Normals_SpecIntens_SpecPower;
	GBuffer1_Normals_SpecIntens_SpecPower = new RenderToTextureBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R16G16B16A16_FLOAT);

	delete GBuffer0_Diffuse;
	GBuffer0_Diffuse = new RenderToTextureBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	delete HDRBackBuffer;
	HDRBackBuffer = new RenderToTextureBuffer(Device, Resolution.x, Resolution.y, DXGI_FORMAT_R16G16B16A16_FLOAT);

	delete WorldShadowmap1;
	int s = Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize;
	WorldShadowmap1 = new RenderToDepthStencilBuffer(Device, s, s, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);

	Engine::AntTweakBar->OnResize(newSize);
	
	return XR_SUCCESS;
}

/** Called when the game wants to render a new frame */
XRESULT D3D11GraphicsEngine::OnBeginFrame()
{
	Engine::GAPI->GetRendererState()->RendererInfo.Timing.StartTotal();

	// Check resolution
	/*RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	if(desktopRect.right *RES_UPSCALE != GetResolution().x || desktopRect.bottom * RES_UPSCALE != GetResolution().y)
	{
		// Match the resolution with the current desktop resolution
		OnResize(INT2(desktopRect.right * RES_UPSCALE, desktopRect.bottom * RES_UPSCALE));
	}*/

		// Enter the critical section for safety while executing the deferred command list
	Engine::GAPI->EnterResourceCriticalSection();
	ID3D11CommandList* dc_cl = NULL;
	DeferredContext->FinishCommandList(true, &dc_cl);

	Engine::GAPI->SetFrameLoadedTexturesReady();
	Engine::GAPI->ClearFrameLoadedTextures();

	Engine::GAPI->LeaveResourceCriticalSection();
	if (dc_cl)
	{
		//LogInfo() << "Executing command list";
		Context->ExecuteCommandList(dc_cl, true);
		dc_cl->Release();
	}

	// Check for editorpanel
	if(!UIView)
	{
		if(Engine::GAPI->GetRendererState()->RendererSettings.EnableEditorPanel)
		{
			UIView = new D2DView;

			ID3D11Texture2D* tex;
			BackbufferRTV->GetResource((ID3D11Resource **)&tex);
			if(XR_SUCCESS != UIView->Init(Resolution, tex))
			{
				delete UIView;
				UIView = NULL;
			}

			if(tex)
				tex->Release();
		}
	}

	// Check for shadowmap resize
	int s = Engine::GAPI->GetRendererState()->RendererSettings.ShadowMapSize;
	float r = 0;
	switch(s)
	{
	case 0:
		s = 512;
		break;

	case 1:
		s = 1024;
		break;

	case 2:
		s = 2048;
		break;

	case 3:
		s = 4096;
		break;
	}

	if(WorldShadowmap1->GetSizeX() != s)
	{
		int old = WorldShadowmap1->GetSizeX();
		LogInfo() << "Shadowmapresolution changed to: " << s << "x" << s;
		delete WorldShadowmap1; WorldShadowmap1 = NULL;
		WorldShadowmap1 = new RenderToDepthStencilBuffer(Device, s, s, DXGI_FORMAT_R32_TYPELESS, NULL, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);

		Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale *= old / (float)s;
	}

	// Force the mode
	zCView::SetMode((int)(Resolution.x / Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale), (int)(Resolution.y / Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale), 32);

	//return XR_SUCCESS;

	// Notify the shader manager
	ShaderManager->OnFrameStart();

	// Clear using the fog color
	//Clear(float4(0,0,0,0));
	
	// Enable blending, in case gothic want's to render its save-screen
	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	UpdateRenderStates();

	// Bind GBuffers
	//ID3D11RenderTargetView* rtvs[] = {GBuffer0_Diffuse->GetRenderTargetView(), GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView()};
	//Context->OMSetRenderTargets(2, rtvs, DepthStencilBuffer->GetDepthStencilView());
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_Ex");

	PS_DiffuseNormalmappedFxMap = ShaderManager->GetPShader("PS_DiffuseNormalmappedFxMap");
	PS_DiffuseNormalmappedAlphatestFxMap = ShaderManager->GetPShader("PS_DiffuseNormalmappedAlphatestFxMap");
	PS_DiffuseNormalmapped = ShaderManager->GetPShader("PS_DiffuseNormalmapped");
	PS_Diffuse = ShaderManager->GetPShader("PS_Diffuse");
	PS_DiffuseNormalmappedAlphatest = ShaderManager->GetPShader("PS_DiffuseNormalmappedAlphaTest");
	PS_DiffuseAlphatest = ShaderManager->GetPShader("PS_DiffuseAlphaTest");
	PS_Simple = ShaderManager->GetPShader("PS_Simple");
	return XR_SUCCESS;
}

/** Called when the game ended it's frame */
XRESULT D3D11GraphicsEngine::OnEndFrame()
{
	Present();

	Engine::GAPI->GetRendererState()->RendererInfo.Timing.StopTotal();

	return XR_SUCCESS;
}

/** Called when the game wants to clear the bound rendertarget */
XRESULT D3D11GraphicsEngine::Clear(const float4& color)
{


	//Context->ClearRenderTargetView(BackbufferRTV, (float *)&D3DXVECTOR4(1,0,0,0));
	Context->ClearDepthStencilView(DepthStencilBuffer->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	
	Context->ClearRenderTargetView(GBuffer0_Diffuse->GetRenderTargetView(), (float *)&D3DXVECTOR4(1,0,0,0));
	Context->ClearRenderTargetView(GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView(), (float *)&D3DXVECTOR4(0,0,0,0));
	Context->ClearRenderTargetView(HDRBackBuffer->GetRenderTargetView(), (float *)&D3DXVECTOR4(0,0,0,0));
	
	return XR_SUCCESS;
}

/** Creates a vertexbuffer object (Not registered inside) */
XRESULT D3D11GraphicsEngine::CreateVertexBuffer(BaseVertexBuffer** outBuffer)
{
	*outBuffer = new D3D11VertexBuffer;
	return XR_SUCCESS;
}

/** Creates a texture object (Not registered inside) */
XRESULT D3D11GraphicsEngine::CreateTexture(BaseTexture** outTexture)
{
	*outTexture = new D3D11Texture;
	return XR_SUCCESS;
}

/** Creates a constantbuffer object (Not registered inside) */
XRESULT D3D11GraphicsEngine::CreateConstantBuffer(BaseConstantBuffer** outCB, void* data, int size)
{
	*outCB = new D3D11ConstantBuffer(size, data);
	return XR_SUCCESS;
}

/** Returns a list of available display modes */
XRESULT D3D11GraphicsEngine::GetDisplayModeList(std::vector<DisplayModeInfo>* modeList, bool includeSuperSampling )
{
	HRESULT hr;
	UINT numModes = 0;
	DXGI_MODE_DESC* displayModes = NULL;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	IDXGIOutput* output;

	// Get desktop rect
	RECT desktop;
	GetClientRect(GetDesktopWindow(), &desktop);

	if(!DXGIAdapter)
		return XR_FAILED;

	DXGIAdapter->EnumOutputs(0, &output);

	if(!output)
		return XR_FAILED;

	hr = output->GetDisplayModeList(format, 0, &numModes, NULL);

	displayModes = new DXGI_MODE_DESC[numModes];

	// Get the list
	hr = output->GetDisplayModeList(format, 0, &numModes, displayModes);

	for (unsigned int i = 0; i<numModes; i++)
	{
		if (displayModes[i].Format != format)
			continue;

		DisplayModeInfo info;
		info.Height = displayModes[i].Height;
		info.Width = displayModes[i].Width;
		info.Bpp = 32;

		if(info.Width > (unsigned long)desktop.right || info.Height > (unsigned long)desktop.bottom)
			continue; // Skip bigger sizes than the desktop rect, because DXGI doesn't like them apparently TODO: Fix this if possible!

		if(!modeList->empty() && memcmp(&modeList->back(), &info, sizeof(info)) == 0)
			continue; // Already have that in list

		modeList->push_back(info);
	}

	if(includeSuperSampling)
	{
		// Put supersampling resolutions in, up to just below 8k
		int i = 2;
		DisplayModeInfo ssBase = modeList->back();
		while(ssBase.Width * i < 8192 && ssBase.Height * i < 8192)
		{
			DisplayModeInfo info;
			info.Height = ssBase.Height * i;
			info.Width = ssBase.Width * i;
			info.Bpp = 32;

			modeList->push_back(info);
			i++;
		}
	}


	delete[] displayModes;

	output->Release();

	return XR_SUCCESS;
}

/** Presents the current frame to the screen */
XRESULT D3D11GraphicsEngine::Present()
{
	//Context->ClearRenderTargetView(PfxRenderer->GetTempBuffer()->GetRenderTargetView(), (float *)&float4(0,0,0,1));
	/*PfxRenderer->CopyTextureToRTV(BackbufferSRV, PfxRenderer->GetTempBuffer()->GetRenderTargetView());
	PfxRenderer->BlurTexture(PfxRenderer->GetTempBuffer());

	//Clear(float4(0,1,0,0));
	PfxRenderer->CopyTextureToRTV(PfxRenderer->GetTempBuffer()->GetShaderResView(), BackbufferRTV);*/

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 0.0f;
	vp.Width = (float)GetBackbufferResolution().x;
	vp.Height = (float)GetBackbufferResolution().y;

	Context->RSSetViewports(1, &vp);


	// Copy HDR scene to backbuffer
	SetDefaultStates();
	SetActivePixelShader("PS_PFX_GammaCorrectInv");
	ActivePS->Apply();

	GammaCorrectConstantBuffer gcb;
	gcb.G_Gamma = Engine::GAPI->GetGammaValue();
	gcb.G_Brightness = Engine::GAPI->GetBrightnessValue();

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&gcb);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), BackbufferRTV, INT2(0,0), true);

	//Context->ClearState();

	Context->OMSetRenderTargets(1, &BackbufferRTV, NULL);

	// Check for movie-frame
	if(Engine::GAPI->GetPendingMovieFrame())
	{
		Engine::GAPI->GetPendingMovieFrame()->BindToPixelShader(0);
		DrawQuad(INT2(0,0), GetBackbufferResolution());

		Engine::GAPI->SetPendingMovieFrame(NULL);
	}

	// Draw ant tweak bar
	SetDefaultStates();
	Engine::AntTweakBar->Draw();
	SetDefaultStates();
	//LineRenderer->ClearCache();

	if(UIView)UIView->Render(Engine::GAPI->GetFrameTimeSec());

	//Engine::GAPI->EnterResourceCriticalSection();
	bool vsync = Engine::GAPI->GetRendererState()->RendererSettings.EnableVSync;
	if(SwapChain->Present(vsync ? 1 : 0, 0) == DXGI_ERROR_DEVICE_REMOVED)
	{
		switch(Device->GetDeviceRemovedReason())
		{
		case DXGI_ERROR_DEVICE_HUNG:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_DEVICE_HUNG)";
			exit(0);
			break;

		case DXGI_ERROR_DEVICE_REMOVED:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_DEVICE_REMOVED)";
			exit(0);
			break;

		case DXGI_ERROR_DEVICE_RESET:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_DEVICE_RESET)";
			exit(0);
			break;

		case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_DRIVER_INTERNAL_ERROR)";
			exit(0);
			break;

		case DXGI_ERROR_INVALID_CALL:
			LogErrorBox() << "Device Removed! (DXGI_ERROR_INVALID_CALL)";
			exit(0);
			break;

		case S_OK:
			LogInfo() << "Device removed, but we're fine!";
			break;

		default:
			LogWarnBox() << "Device Removed! (Unknown reason)";
		}
	}
	//Engine::GAPI->LeaveResourceCriticalSection();	

	PresentPending = false;

	return XR_SUCCESS;
}

/** Called to set the current viewport */
XRESULT D3D11GraphicsEngine::SetViewport(const ViewportInfo& viewportInfo)
{
	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = (float)viewportInfo.TopLeftX;
	viewport.TopLeftY = (float)viewportInfo.TopLeftY;
	viewport.Width = (float)viewportInfo.Width;
	viewport.Height = (float)viewportInfo.Height;
	viewport.MinDepth = viewportInfo.MinZ;
	viewport.MaxDepth = viewportInfo.MaxZ;

	Context->RSSetViewports(1, &viewport);

	return XR_SUCCESS;
}

/** Draws a vertexbuffer, non-indexed (World)*/
XRESULT D3D11GraphicsEngine::DrawVertexBuffer(BaseVertexBuffer* vb, unsigned int numVertices, unsigned int stride)
{
	UINT offset = 0;
	UINT uStride = stride;
	ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
	Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	//Draw the mesh
	Context->Draw(numVertices, 0);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numVertices;

	return XR_SUCCESS;
}

/** Draws a vertexbuffer, non-indexed (VOBs)*/
XRESULT D3D11GraphicsEngine::DrawVertexBufferIndexed(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset)
{
	if(vb)
	{
		UINT offset = 0;
		UINT uStride = sizeof(ExVertexStruct);
		ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
		Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

		if(sizeof(VERTEX_INDEX) == sizeof(unsigned short))
		{
			Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);
		}else
		{
			Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		}
	}

	if(numIndices)
	{
		//Draw the mesh
		Context->DrawIndexed(numIndices, indexOffset, 0);

		Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numIndices / 3;
	}
	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::DrawVertexBufferIndexedUINT(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, unsigned int indexOffset)
{
	if(vb)
	{
		UINT offset = 0;
		UINT uStride = sizeof(ExVertexStruct);
		ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
		Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
		
	}

	if(numIndices)
	{
		//Draw the mesh
		Context->DrawIndexed(numIndices, indexOffset, 0);

		Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numIndices / 3;
	}

	return XR_SUCCESS;
}

/** Binds viewport information to the given constantbuffer slot */
XRESULT D3D11GraphicsEngine::BindViewportInformation(const std::string& shader, int slot) 
{
	D3D11_VIEWPORT vp;
	UINT num = 1;
	Context->RSGetViewports(&num, &vp);

	// Update viewport information
	float scale = Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale;
	Temp2Float2[0].x = vp.TopLeftX / scale;
	Temp2Float2[0].y = vp.TopLeftY / scale;
	Temp2Float2[1].x = vp.Width / scale;
	Temp2Float2[1].y = vp.Height / scale;

	D3D11PShader* ps = ShaderManager->GetPShader(shader);
	D3D11VShader* vs = ShaderManager->GetVShader(shader);

	if(vs)
	{
		vs->GetConstantBuffer()[slot]->UpdateBuffer(Temp2Float2);
		vs->GetConstantBuffer()[slot]->BindToVertexShader(slot);
	}

	if(ps)
	{
		ps->GetConstantBuffer()[slot]->UpdateBuffer(Temp2Float2);
		ps->GetConstantBuffer()[slot]->BindToVertexShader(slot);
	}

	return XR_SUCCESS;
}

/** Draws a vertexarray, non-indexed (HUD, 2D)*/
XRESULT D3D11GraphicsEngine::DrawVertexArray(ExVertexStruct* vertices, unsigned int numVertices, unsigned int startVertex, unsigned int stride)
{
	UpdateRenderStates();
	D3D11VShader* vShader = ActiveVS;//ShaderManager->GetVShader("VS_TransformedEx");
	
	// Bind the FF-Info to the first PS slot

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	SetupVS_ExMeshDrawCall();

	D3D11_BUFFER_DESC desc;
	((D3D11VertexBuffer *)TempVertexBuffer)->GetVertexBuffer()->GetDesc(&desc);

	if(desc.ByteWidth < stride * numVertices)
	{
		LogInfo() << "TempVertexBuffer too small (" << desc.ByteWidth << "), need " << stride * numVertices << " bytes. Recreating buffer.";

		// Buffer too small, recreate it
		delete TempVertexBuffer;
		TempVertexBuffer = new D3D11VertexBuffer();

		TempVertexBuffer->Init(NULL, stride * numVertices, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	}

	TempVertexBuffer->UpdateBuffer(vertices, stride * numVertices);

	UINT offset = 0;
	UINT uStride = stride;
	ID3D11Buffer* buffer = TempVertexBuffer->GetVertexBuffer();
	Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	//Draw the mesh
	Context->Draw(numVertices, startVertex);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numVertices;

	return XR_SUCCESS;
}

/** Draws a vertexbuffer, non-indexed, binding the FF-Pipe values */
XRESULT D3D11GraphicsEngine::DrawVertexBufferFF(BaseVertexBuffer* vb, unsigned int numVertices, unsigned int startVertex, unsigned int stride)
{
	SetupVS_ExMeshDrawCall();

	// Bind the FF-Info to the first PS slot
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	UINT offset = 0;
	UINT uStride = stride;
	ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
	Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	//Draw the mesh
	Context->Draw(numVertices, startVertex);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numVertices;

	return XR_SUCCESS;
}

/** Draws a skeletal mesh */
XRESULT D3D11GraphicsEngine::DrawSkeletalMesh(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, const std::vector<D3DXMATRIX>& transforms, float fatness, SkeletalMeshVisualInfo* msh)
{
	Context->RSSetState(WorldRasterizerState);
	Context->OMSetDepthStencilState(DefaultDepthStencilState, 0);
	SetActiveVertexShader("VS_ExSkeletal");

	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_World");
	D3D11PShader* defaultPS = ActivePS;

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetProjectionMatrix();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Get currently bound texture name
	zCTexture* tex = Engine::GAPI->GetBoundTexture(0);

	if(tex)
	{
		MaterialInfo* info = Engine::GAPI->GetMaterialInfoFrom(tex);
		if(!info->Constantbuffer)
			info->UpdateConstantbuffer();

		info->Constantbuffer->BindToPixelShader(2);

		BindShaderForTexture(tex);
	
		if(RenderingStage == DES_MAIN)
		{
			if(msh && msh->TesselationInfo.buffer.VT_TesselationFactor > 0.0f)
			{
				MyDirectDrawSurface7* surface = tex->GetSurface();
				ID3D11ShaderResourceView* srv = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
				// Set normal/displacement map
				Context->DSSetShaderResources(0,1, &srv);
				Context->HSSetShaderResources(0,1, &srv);
				Setup_PNAEN(PNAEN_Skeletal);
				msh->TesselationInfo.Constantbuffer->BindToDomainShader(1);
				msh->TesselationInfo.Constantbuffer->BindToHullShader(1);
			}else if(ActiveHDS)
			{
				Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				Context->DSSetShader(NULL, NULL, NULL);
				Context->HSSetShader(NULL, NULL, NULL);
				ActiveHDS = NULL;
			}
		}
	}



	VS_ExConstantBuffer_PerInstanceSkeletal cb2;
	cb2.World = world;
	cb2.PI_ModelFatness = fatness;

	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cb2);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	// Copy bones
	memcpy(TempBonesD3DXmatrix, &transforms[0], sizeof(D3DXMATRIX) * std::min(transforms.size(), sizeof(TempBonesD3DXmatrix) / sizeof(TempBonesD3DXmatrix[0])));
	ActiveVS->GetConstantBuffer()[2]->UpdateBuffer(TempBonesD3DXmatrix);
	ActiveVS->GetConstantBuffer()[2]->BindToVertexShader(2);

	if(transforms.size() >= sizeof(TempBonesD3DXmatrix) / sizeof(TempBonesD3DXmatrix[0]))
	{
		LogWarn() << "SkeletalMesh has more than " << sizeof(TempBonesD3DXmatrix) / sizeof(TempBonesD3DXmatrix[0]) << " bones! (" << transforms.size() << ")Up this limit!";
	}

	ActiveVS->Apply();
	ActivePS->Apply();

	
	UINT offset = 0;
	UINT uStride = sizeof(ExSkelVertexStruct);
	ID3D11Buffer* buffer = ((D3D11VertexBuffer *)vb)->GetVertexBuffer();
	Context->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	if(sizeof(VERTEX_INDEX) == sizeof(unsigned short))
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);
	}else
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	if(RenderingStage == DES_SHADOWMAP)
	{
		// Unbind PixelShader in this case
		Context->PSSetShader(NULL, NULL, NULL);
		ActivePS = NULL;
	}

	//Draw the mesh
	Context->DrawIndexed(numIndices, 0, 0);

	if(ActiveHDS && RenderingStage == DES_MAIN)
	{
		Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Context->DSSetShader(NULL, NULL, NULL);
		Context->HSSetShader(NULL, NULL, NULL);
		ActiveHDS = NULL;
	}

	return XR_SUCCESS;
}

/** Draws a batch of instanced geometry */
XRESULT D3D11GraphicsEngine::DrawInstanced(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, void* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride)
{
	UpdateRenderStates();

	// Check buffersize
	D3D11_BUFFER_DESC desc;
	((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()->GetDesc(&desc);

	if(desc.ByteWidth < instanceDataStride * numInstances)
	{
		LogInfo() << "Instancing buffer too small (" << desc.ByteWidth << "), need " << instanceDataStride * numInstances << " bytes. Recreating buffer.";

		// Buffer too small, recreate it
		delete DynamicInstancingBuffer;
		DynamicInstancingBuffer = new D3D11VertexBuffer();

		// Put in some little extra space (16) so we don't need to recreate this every frame when approaching a field of stones or something.
		DynamicInstancingBuffer->Init(NULL, instanceDataStride * (numInstances + 16), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
	}

	// Update the vertexbuffer
	DynamicInstancingBuffer->UpdateBuffer(instanceData, instanceDataStride * numInstances);

	// Bind shader and pipeline flags
	//Context->RSSetState(WorldRasterizerState);
	//Context->OMSetDepthStencilState(DefaultDepthStencilState, 0);
	D3D11VShader* vShader = ShaderManager->GetVShader("VS_ExInstanced");

	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetProjectionMatrix();

	
	VS_ExConstantBuffer_PerFrame cb;
	cb.View = view;
	cb.Projection = proj;

	VS_ExConstantBuffer_PerInstance cb2;
	cb2.World = world;




	vShader->GetConstantBuffer()[0]->UpdateBuffer(&cb);
	vShader->GetConstantBuffer()[0]->BindToVertexShader(0);

	//vShader->GetConstantBuffer()[1]->UpdateBuffer(&cb2);
	//vShader->GetConstantBuffer()[1]->BindToVertexShader(1);

	vShader->Apply();
	
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT offset[] = {0,0};
	UINT uStride[] = {vertexStride, instanceDataStride};
	ID3D11Buffer* buffers[] = {((D3D11VertexBuffer *)vb)->GetVertexBuffer(),
		((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()};
	Context->IASetVertexBuffers(0, 2, buffers, uStride, offset);

	if(sizeof(VERTEX_INDEX) == sizeof(unsigned short))
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);
	}else
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	//Draw the batch
	Context->DrawIndexedInstanced(numIndices, numInstances, 0, 0, 0);

	return XR_SUCCESS;
}

/** Draws a batch of instanced geometry */
XRESULT D3D11GraphicsEngine::DrawInstanced(BaseVertexBuffer* vb, BaseVertexBuffer* ib, unsigned int numIndices, BaseVertexBuffer* instanceData, unsigned int instanceDataStride, unsigned int numInstances, unsigned int vertexStride, unsigned int startInstanceNum, unsigned int indexOffset)
{
	// Bind shader and pipeline flags
	UINT offset[] = {0,0};
	UINT uStride[] = {vertexStride, instanceDataStride};
	ID3D11Buffer* buffers[] = {((D3D11VertexBuffer *)vb)->GetVertexBuffer(),
		((D3D11VertexBuffer *)instanceData)->GetVertexBuffer()};
	Context->IASetVertexBuffers(0, 2, buffers, uStride, offset);

	if(sizeof(VERTEX_INDEX) == sizeof(unsigned short))
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R16_UINT, 0);
	}else
	{
		Context->IASetIndexBuffer(((D3D11VertexBuffer *)ib)->GetVertexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	}

	unsigned int max = Engine::GAPI->GetRendererState()->RendererSettings.MaxNumFaces * 3;
	numIndices = max != 0 ? (numIndices < max ? numIndices : max) : numIndices;

	//Draw the batch
	Context->DrawIndexedInstanced(numIndices, numInstances, indexOffset, 0, startInstanceNum);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnVobs++;

	return XR_SUCCESS;
}

/** Sets the active pixel shader object */
XRESULT D3D11GraphicsEngine::SetActivePixelShader(const std::string& shader)
{
	ActivePS = ShaderManager->GetPShader(shader);

	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::SetActiveVertexShader(const std::string& shader)
{
	ActiveVS = ShaderManager->GetVShader(shader);

	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::SetActiveHDShader(const std::string& shader)
{
	ActiveHDS = ShaderManager->GetHDShader(shader);

	return XR_SUCCESS;
}

/** Binds the active PixelShader */
XRESULT D3D11GraphicsEngine::BindActivePixelShader()
{
	if(ActiveVS)ActiveVS->Apply();
	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::BindActiveVertexShader()
{
	if(ActivePS)ActivePS->Apply();
	return XR_SUCCESS;
}

/** Unbinds the texture at the given slot */
XRESULT D3D11GraphicsEngine::UnbindTexture(int slot)
{
	ID3D11ShaderResourceView* srv = NULL;
	Context->PSSetShaderResources(slot, 1, &srv);
	Context->VSSetShaderResources(slot, 1, &srv);

	return XR_SUCCESS;
}

/** Recreates the renderstates */
XRESULT D3D11GraphicsEngine::UpdateRenderStates()
{
	if (Engine::GAPI->GetRendererState()->BlendState.StateDirty)
	{
		D3D11BlendStateInfo* state = (D3D11BlendStateInfo *)GothicStateCache::s_BlendStateMap[Engine::GAPI->GetRendererState()->BlendState];

		if(!state)
		{
			// Create new state
			state = new D3D11BlendStateInfo(Engine::GAPI->GetRendererState()->BlendState);

			GothicStateCache::s_BlendStateMap[Engine::GAPI->GetRendererState()->BlendState] = state;
		}

		FFBlendState = state->State;

		Engine::GAPI->GetRendererState()->BlendState.StateDirty = false;
		Context->OMSetBlendState(FFBlendState, (float *)&D3DXVECTOR4(0, 0, 0, 0), 0xFFFFFFFF);
	}

	

	if (Engine::GAPI->GetRendererState()->RasterizerState.StateDirty)
	{
		D3D11RasterizerStateInfo* state = (D3D11RasterizerStateInfo *)GothicStateCache::s_RasterizerStateMap[Engine::GAPI->GetRendererState()->RasterizerState];

		if(!state)
		{
			// Create new state
			state = new D3D11RasterizerStateInfo(Engine::GAPI->GetRendererState()->RasterizerState);

			GothicStateCache::s_RasterizerStateMap[Engine::GAPI->GetRendererState()->RasterizerState] = state;
		}

		FFRasterizerState = state->State;

		Engine::GAPI->GetRendererState()->RasterizerState.StateDirty = false;
		Context->RSSetState(FFRasterizerState);
	}

	

	if (Engine::GAPI->GetRendererState()->DepthState.StateDirty)
	{
		D3D11DepthBufferState* state = (D3D11DepthBufferState *)GothicStateCache::s_DepthBufferMap[Engine::GAPI->GetRendererState()->DepthState];

		if(!state)
		{
			// Create new state
			state = new D3D11DepthBufferState(Engine::GAPI->GetRendererState()->DepthState);

			GothicStateCache::s_DepthBufferMap[Engine::GAPI->GetRendererState()->DepthState] = state;
		}

		FFDepthStencilState = state->State;

		Engine::GAPI->GetRendererState()->DepthState.StateDirty = false;
		Context->OMSetDepthStencilState(FFDepthStencilState, 0);
	}

	

	return XR_SUCCESS;
}

/** Called when we started to render the world */
XRESULT D3D11GraphicsEngine::OnStartWorldRendering()
{
	//Clear(float4(0,0,0,0));
	//Clear(float4(0xFF44AEFF));

	SetDefaultStates();

	if(Engine::GAPI->GetRendererState()->RendererSettings.DisableRendering)
		return XR_SUCCESS;

	//return XR_SUCCESS;
	if(PresentPending)
		return XR_SUCCESS;

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = (float)GetResolution().x;
	vp.Height = (float)GetResolution().y;

	Context->RSSetViewports(1, &vp);

	ID3D11RenderTargetView* rtvs[] = {GBuffer0_Diffuse->GetRenderTargetView(), GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView()};
	Context->OMSetRenderTargets(2, rtvs, DepthStencilBuffer->GetDepthStencilView());

	Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);

	Clear(float4(Engine::GAPI->GetRendererState()->GraphicsState.FF_FogColor, 0.0f));

	// Clear textures from the last frame
	FrameTextures.clear(); 
	RenderedVobs.resize(0); // Keep memory allocated on this
	FrameWaterSurfaces.clear();
	FrameTransparencyMeshes.clear();

	// Update view distances
	InfiniteRangeConstantBuffer->UpdateBuffer(&D3DXVECTOR4(FLT_MAX, 0, 0, 0));
	OutdoorSmallVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius, 0, 0, 0));
	OutdoorVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius, 0, 0, 0));


	// Update editor
	if(UIView)UIView->Update(Engine::GAPI->GetFrameTimeSec());

	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = false;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawSky && Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetBspTreeMode() == zBSP_MODE_OUTDOOR)
	{
		// Draw back of the sky if outdoor
		DrawSky();
	}

	// Draw world
	Engine::GAPI->DrawWorldMeshNaive();

	

	// Draw HBAO
	if(Engine::GAPI->GetRendererState()->RendererSettings.HbaoSettings.Enabled)
		PfxRenderer->DrawHBAO(HDRBackBuffer->GetRenderTargetView());

	SetDefaultStates();
	

	//PfxRenderer->RenderDistanceBlur();
	
	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_Ex");

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawSky)
	{
		// Draw clouds
		//Draw3DClouds();
	}

	

	// Draw water surfaces of current frame
	DrawWaterSurfaces();

	// Draw light-shafts
	DrawMeshInfoListAlphablended(FrameTransparencyMeshes);

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawFog && 
		Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetBspTreeMode() == zBSP_MODE_OUTDOOR)
		PfxRenderer->RenderHeightfog();


	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_Ex");

	SetDefaultStates();

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	// Draw unlit decals //FIXME: Only get them once!
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawParticleEffects)
	{
		std::vector<zCVob *> decals;
		Engine::GAPI->GetVisibleDecalList(decals);

		// Draw stuff like candle-flames
		DrawDecalList(decals, false);
	}

	//DrawParticleEffects();
	Engine::GAPI->DrawParticlesSimple();

	// Draw debug lines
	LineRenderer->Flush();
	
	PfxRenderer->RenderGodRays();

	if(Engine::GAPI->GetRendererState()->RendererSettings.EnableHDR)
		PfxRenderer->RenderHDR();

	if(Engine::GAPI->GetRendererState()->RendererSettings.EnableSMAA)
		PfxRenderer->RenderSMAA();

	//PfxRenderer->CopyTextureToRTV(GBuffer1_Normals_SpecIntens_SpecPower->GetShaderResView(), BackbufferRTV, INT2(Resolution.x / 4, Resolution.y / 4));

	// Disable the depth-buffer
	Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled = false;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	PresentPending = true;


	// Set viewport for gothics rendering
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = (float)GetBackbufferResolution().x;
	vp.Height = (float)GetBackbufferResolution().y;

	Context->RSSetViewports(1, &vp);

	// If we currently are underwater, then draw underwater effects
	if(Engine::GAPI->IsUnderWater())
		DrawUnderwaterEffects();

	// Clear here to get a working depthbuffer but no interferences with world geometry for gothic UI-Rendering
	Context->ClearDepthStencilView(DepthStencilBuffer->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0.0f);
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), NULL);

	SetDefaultStates();

	return XR_SUCCESS;
}


void D3D11GraphicsEngine::SetupVS_ExMeshDrawCall()
{
	UpdateRenderStates();

	if(ActiveVS)ActiveVS->Apply();
	if(ActivePS)ActivePS->Apply();

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void D3D11GraphicsEngine::SetupVS_ExConstantBuffer()
{
	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetProjectionMatrix();

	VS_ExConstantBuffer_PerFrame cb;
	cb.View = view;
	cb.Projection = proj;
	cb.ViewProj = proj * view;

	ActiveVS->GetConstantBuffer()[0]->UpdateBuffer(&cb);
	ActiveVS->GetConstantBuffer()[0]->BindToVertexShader(0);
	ActiveVS->GetConstantBuffer()[0]->BindToDomainShader(0);
	ActiveVS->GetConstantBuffer()[0]->BindToHullShader(0);
}

void D3D11GraphicsEngine::SetupVS_ExPerInstanceConstantBuffer()
{
	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;

	VS_ExConstantBuffer_PerInstance cb;
	cb.World = world;

	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cb);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);
}

/** Puts the current world matrix into a CB and binds it to the given slot */
void D3D11GraphicsEngine::SetupPerInstanceConstantBuffer(int slot)
{
	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;

	VS_ExConstantBuffer_PerInstance cb;
	cb.World = world;

	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cb);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(slot);
}

bool SectionRenderlistSortCmp(std::pair<float, WorldMeshSectionInfo*>& a, std::pair<float, WorldMeshSectionInfo*>& b)
{
	return a.first < b.first;
}

/** Test draw world */
void D3D11GraphicsEngine::TestDrawWorldMesh()
{
	std::list<WorldMeshSectionInfo*> renderList;

	Engine::GAPI->CollectVisibleSections(renderList);

	DistortionTexture->BindToPixelShader(0);

	
	DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

	for(std::list<WorldMeshSectionInfo*>::iterator itr = renderList.begin(); itr != renderList.end(); itr++)
	{

		for(std::map<BaseTexture *, std::vector<MeshInfo*>>::iterator it = (*itr)->WorldMeshesByCustomTexture.begin(); it != (*itr)->WorldMeshesByCustomTexture.end();it++)
		{
			if((*it).first)
			{
				(*it).first->BindToPixelShader(0);
			}

			for(unsigned int i=0;i<(*it).second.size();i++)
			{
				// Draw from wrapped mesh
				DrawVertexBufferIndexedUINT(NULL, NULL, (*it).second[i]->Indices.size(), (*it).second[i]->BaseIndexLocation);		
			}
		}

		for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator it = (*itr)->WorldMeshesByCustomTextureOriginal.begin(); it != (*itr)->WorldMeshesByCustomTextureOriginal.end();it++)
		{
			if((*it).first && (*it).first->GetTexture())
			{
				if((*it).first->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
					(*it).first->GetTexture()->Bind(0);
				else
					continue;
			}

			for(unsigned int i=0;i<(*it).second.size();i++)
			{
				// Draw from wrapped mesh
				DrawVertexBufferIndexedUINT(NULL, NULL, (*it).second[i]->Indices.size(), (*it).second[i]->BaseIndexLocation);		
			}
		}
	}

	/*int x=0;
	for(stdext::hash_map<zCTexture*, std::vector<MeshInfo *>>::iterator it = meshesByTexture.begin();it != meshesByTexture.end(); it++)
	{
			
	}*/

}

/** Draws a list of mesh infos */
XRESULT D3D11GraphicsEngine::DrawMeshInfoListAlphablended(const std::vector<std::pair<MeshKey, MeshInfo*>>& list)
{
	SetDefaultStates();

	// Setup renderstates
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();


	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);
	Engine::GAPI->ResetWorldTransform();

	SetActivePixelShader("PS_Diffuse");
	SetActiveVertexShader("VS_Ex");

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	// Bind wrapped mesh vertex buffers
	DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

	int lastAlphaFunc = 0;

	// Draw the list
	for(auto it = list.begin(); it != list.end(); it++)
	{
		int indicesNumMod = 1;
		if((*it).first.Texture != NULL)
		{
			MyDirectDrawSurface7* surface = (*it).first.Texture->GetSurface();
			ID3D11ShaderResourceView* srv[3];
			
			// Get diffuse and normalmap
			srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
			srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
			srv[2] = surface->GetFxMap() ? ((D3D11Texture *)surface->GetFxMap())->GetShaderResourceView() : NULL;


			// Bind both
			Context->PSSetShaderResources(0,3, srv);

			// Get the right shader for it

			BindShaderForTexture((*it).first.Texture, false, (*it).first.Material->GetAlphaFunc());
			
			// Check for alphablending on world mesh
			if(lastAlphaFunc != (*it).first.Material->GetAlphaFunc())
			{
				if((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND)
					Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();

				if((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD)
					Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();

				Engine::GAPI->GetRendererState()->BlendState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
				lastAlphaFunc = (*it).first.Material->GetAlphaFunc();
			}


			MaterialInfo* info = (*it).first.Info;
			if(!info->Constantbuffer)
				info->UpdateConstantbuffer();
			
			info->Constantbuffer->BindToPixelShader(2);

			// Don't let the game unload the texture after some time
			(*it).first.Texture->CacheIn(0.6f);

			// Draw the section-part
			DrawVertexBufferIndexedUINT(NULL, NULL, 
				(*it).second->Indices.size(),
				(*it).second->BaseIndexLocation);
		}
	}

	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngine::DrawWorldMesh(bool noTextures)
{
	if(!Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
		return XR_SUCCESS;

	SetDefaultStates();

	// Setup renderstates
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();


	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);
	Engine::GAPI->ResetWorldTransform();

	SetActivePixelShader("PS_Diffuse");
	SetActiveVertexShader("VS_Ex");

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	std::list<WorldMeshSectionInfo*> renderList;
	Engine::GAPI->CollectVisibleSections(renderList);

	DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, 
				Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

	std::list<std::pair<MeshKey, WorldMeshInfo *>> meshList;

	int numUncachedTextures = 0;
	for(int i=0;i<2;i++)
	{
		for(std::list<WorldMeshSectionInfo*>::iterator it = renderList.begin(); it != renderList.end(); it++)
		{
			for(std::map<MeshKey, WorldMeshInfo*>::iterator itm = (*it)->WorldMeshes.begin(); itm != (*it)->WorldMeshes.end();itm++)
			{
				if((*itm).first.Material)
				{
					zCTexture* aniTex = (*itm).first.Material->GetTexture();

					if(!aniTex)
						continue;

					if(i == 1) // Second try, this is only true if we have many unloaded textures
					{
						aniTex->CacheIn(-1);
					}else
					{
						if(!aniTex->GetSurface() || !aniTex->GetSurface()->GetEngineTexture())
						{
							aniTex->CacheIn(0.6f);
							numUncachedTextures++;
							continue;
						}
					}

					// Check surface type
					if((*itm).first.Info->MaterialType == MaterialInfo::MT_Water)
					{
						FrameWaterSurfaces[(*itm).first.Texture].push_back((*itm).second);
						continue;
					}

					// Check if the animated texture and the registered textures are the same
					if((*itm).first.Texture != aniTex)
					{
						MeshKey key = (*itm).first;
						key.Texture = aniTex;

						// Check for alphablending
						if((*itm).first.Material->GetAlphaFunc() > zMAT_ALPHA_FUNC_FUNC_NONE)
						{
							FrameTransparencyMeshes.push_back((*itm));
						}else
						{
							// Create a new pair using the animated texture
							meshList.push_back(std::make_pair(key, (*itm).second));
						}
						
					}else
					{
						// Check for alphablending
						if((*itm).first.Material->GetAlphaFunc() > zMAT_ALPHA_FUNC_FUNC_NONE)
						{
							FrameTransparencyMeshes.push_back((*itm));
						}else
						{
							// Push this texture/mesh combination
							meshList.push_back((*itm));
						}				
					}			
				}
			}
		}

		if(numUncachedTextures < NUM_UNLOADEDTEXCOUNT_FORCE_LOAD_TEXTURES)
			break;

		// If we get here, there are many unloaded textures.
		// Clear the list and try again, with forcing the textures to load
		meshList.clear();
	}

	struct cmpstruct
	{
		static bool cmp(const std::pair<MeshKey, MeshInfo *>& a, const std::pair<MeshKey, MeshInfo *>& b)
		{
			if(a.first.Texture->HasAlphaChannel())
				return false; // Render alpha last

			if(b.first.Texture->HasAlphaChannel())
				return true; // Render alpha last

			return a.first.Texture < b.first.Texture;
		}
	};

	// Sort by texture
	meshList.sort(cmpstruct::cmp);

	// Draw depth only
	if(Engine::GAPI->GetRendererState()->RendererSettings.DoZPrepass)
	{
		INT2 camSection = WorldConverter::GetSectionOfPos(Engine::GAPI->GetCameraPosition());
		Context->PSSetShader(NULL, NULL, NULL);

		for(std::list<std::pair<MeshKey, WorldMeshInfo *>>::iterator it = meshList.begin(); it != meshList.end(); it++)
		{
			if(!(*it).first.Texture)
				continue;

			if((*it).first.Texture->HasAlphaChannel())
				continue; // Don't pre-render stuff with alpha channel

			if((*it).first.Info->MaterialType == MaterialInfo::MT_Water)
				continue; // Don't pre-render water

			if((*it).second->TesselationSettings.buffer.VT_TesselationFactor > 0.0f)
				continue; // Don't pre-render tesselated surfaces

			DrawVertexBufferIndexedUINT(NULL, NULL, 
				(*it).second->Indices.size(),
				(*it).second->BaseIndexLocation);
		}
	}

	SetActivePixelShader("PS_Diffuse");
	ActivePS->Apply();

	// Now draw the actual pixels
	zCTexture* bound = NULL;
	MaterialInfo* boundInfo = NULL;
	ID3D11ShaderResourceView* boundNormalmap = NULL;
	for(std::list<std::pair<MeshKey, WorldMeshInfo *>>::iterator it = meshList.begin(); it != meshList.end(); it++)
	{
		int indicesNumMod = 1;
		if((*it).first.Texture != bound)
		{
			MyDirectDrawSurface7* surface = (*it).first.Texture->GetSurface();
			ID3D11ShaderResourceView* srv[3];
			
			// Get diffuse and normalmap
			srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
			srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
			srv[2] = surface->GetFxMap() ? ((D3D11Texture *)surface->GetFxMap())->GetShaderResourceView() : NULL;

			boundNormalmap = srv[1];

			// Bind both
			Context->PSSetShaderResources(0,3, srv);

			// Get the right shader for it

			BindShaderForTexture((*it).first.Texture, false, (*it).first.Material->GetAlphaFunc());
			
			// Check for alphablending on world mesh
			if(((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND || (*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD) && !Engine::GAPI->GetRendererState()->BlendState.BlendEnabled)
			{
				if((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND)
					Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();

				if((*it).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD)
					Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();

				Engine::GAPI->GetRendererState()->BlendState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
			}else if(Engine::GAPI->GetRendererState()->BlendState.BlendEnabled && (*it).first.Material->GetAlphaFunc() != zMAT_ALPHA_FUNC_BLEND)
			{
				Engine::GAPI->GetRendererState()->BlendState.SetDefault();
				Engine::GAPI->GetRendererState()->BlendState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = true;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
			}

			MaterialInfo* info = (*it).first.Info;
			if(!info->Constantbuffer)
				info->UpdateConstantbuffer();
			
			info->Constantbuffer->BindToPixelShader(2);

			// Don't let the game unload the texture after some timep
			(*it).first.Texture->CacheIn(0.6f);

			boundInfo = info;
			bound = (*it).first.Texture;

			// Bind normalmap to HDS
			if((*it).second->MeshIndexBufferPNAEN)
			{
				Context->DSSetShaderResources(0,1, &boundNormalmap);
				Context->HSSetShaderResources(0,1, &boundNormalmap);
			}

			/*if((*it).second->TesselationSettings.buffer.VT_TesselationFactor > 0.0f)
			{
				// Set normal/displacement map
				Context->DSSetShaderResources(0,1, &srv[1]);
				Context->HSSetShaderResources(0,1, &srv[1]);
				Setup_PNAEN(PNAEN_Default);

				(*it).second->TesselationSettings.Constantbuffer->BindToDomainShader(1);
				(*it).second->TesselationSettings.Constantbuffer->BindToHullShader(1);
			}else if(ActiveHDS)
			{
				Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				Context->DSSetShader(NULL, NULL, NULL);
				Context->HSSetShader(NULL, NULL, NULL);
				ActiveHDS = NULL;
				SetActiveVertexShader("VS_Ex");
				ActiveVS->Apply();
			}*/
		}

		// Check for tesselated mesh
		if(!ActiveHDS && (*it).second->TesselationSettings.buffer.VT_TesselationFactor > 0.0f)
		{
			// Set normal/displacement map
			Context->DSSetShaderResources(0,1, &boundNormalmap);
			Context->HSSetShaderResources(0,1, &boundNormalmap);
			Setup_PNAEN(PNAEN_Default);			
		}

		// Bind infos for this mesh
		if((*it).second->TesselationSettings.buffer.VT_TesselationFactor > 0.0f)
		{
			(*it).second->TesselationSettings.Constantbuffer->BindToDomainShader(1);
			(*it).second->TesselationSettings.Constantbuffer->BindToHullShader(1);
		}else if(ActiveHDS) // Unbind tesselation-shaders if the mesh doesn't support it
		{
			Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			Context->DSSetShader(NULL, NULL, NULL);
			Context->HSSetShader(NULL, NULL, NULL);
			ActiveHDS = NULL;
			SetActiveVertexShader("VS_Ex");
			ActiveVS->Apply();

			// Bind wrapped mesh again
			DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, 
				Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);
		}

		if(!ActiveHDS)
		{
			// Draw the section-part
			DrawVertexBufferIndexedUINT(NULL, NULL, 
				(*it).second->Indices.size(),
				(*it).second->BaseIndexLocation);
		}else
		{
			// Draw from mesh info
			DrawVertexBufferIndexed((*it).second->MeshVertexBuffer, (*it).second->MeshIndexBufferPNAEN, (*it).second->IndicesPNAEN.size());
		}

	}

	SetDefaultStates();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();


	return XR_SUCCESS;
}

/** Draws the world mesh */
XRESULT D3D11GraphicsEngine::DrawWorldMeshW(bool noTextures)
{
	if(!Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
		return XR_SUCCESS;


	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();

	//Engine::GAPI->SetFarPlane(DEFAULT_FAR_PLANE);

	INT2 camSection = WorldConverter::GetSectionOfPos(camPos);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	// Setup renderstates
	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	

	Engine::GAPI->ResetWorldTransform();
	Engine::GAPI->SetViewTransform(view);

	// Set shader
	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_World");
	D3D11PShader* defaultPS = ActivePS;
	SetActiveVertexShader("VS_Ex");
	D3D11VShader* vsEx = ActiveVS;

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeWorld)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
	}

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

/*#ifdef BUILD_GOTHIC_1_08k
	TestDrawWorldMesh();
	return XR_SUCCESS;
#endif*/

	int numSections = 0;
	int sectionViewDist = Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius;

	std::list<WorldMeshSectionInfo*> renderList;

	Engine::GAPI->CollectVisibleSections(renderList);

	// Sort the list to help pixel-culling
	//renderList.sort(SectionRenderlistSortCmp);


	//noTextures = true;

	// Static, so we can clear the lists but leave the hashmap intact
	static std::hash_map<zCTexture*, std::pair<MaterialInfo*, std::vector<WorldMeshInfo*>>> meshesByMaterial;
	static zCMesh* startMesh = Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetMesh();

	if(startMesh != Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetMesh())
	{
		meshesByMaterial.clear(); // Happens on world change
		startMesh = Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetMesh();
	}

	std::vector<MeshInfo*> WaterSurfaces;

	for(std::list<WorldMeshSectionInfo*>::iterator itr = renderList.begin(); itr != renderList.end(); itr++)
	{
		numSections++;

		//GetLineRenderer()->AddAABBMinMax((*itr)->BoundingBox.Min, (*itr)->BoundingBox.Max);

		// Draw world mesh
		/*DistortionTexture->BindToPixelShader(0);
		if((*itr)->FullStaticMesh)
			Engine::GAPI->DrawMeshInfo(NULL, (*itr)->FullStaticMesh);

		continue;*/

		for(std::map<MeshKey, WorldMeshInfo*>::iterator it = (*itr)->WorldMeshes.begin(); it != (*itr)->WorldMeshes.end();it++)
		{
			if((*it).first.Material)
			{
				std::pair<MaterialInfo*, std::vector<WorldMeshInfo*>>& p = meshesByMaterial[(*it).first.Material->GetTexture()];
				p.second.push_back((*it).second);

				if(!p.first)
				{
					p.first = Engine::GAPI->GetMaterialInfoFrom((*it).first.Material->GetTextureSingle());
				}
			}
			else
			{
				meshesByMaterial[NULL].second.push_back((*it).second);
				meshesByMaterial[NULL].first = Engine::GAPI->GetMaterialInfoFrom(NULL);
			}
		}
	}

	// Bind wrapped mesh vertex buffers
	DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

	for(std::hash_map<zCTexture*, std::pair<MaterialInfo*, std::vector<WorldMeshInfo*>>>::iterator it = meshesByMaterial.begin(); it != meshesByMaterial.end();it++)
	{
		if((*it).second.second.empty())
			continue;

		if(!(*it).first || !(*it).first)
		{
			DistortionTexture->BindToPixelShader(0);
		}else
		{
			//FrameTextures.insert((*it).first);

			MaterialInfo* info = (*it).second.first;
			if(!info->Constantbuffer)
				info->UpdateConstantbuffer();
			
			// Check surface type
			if(info->MaterialType == MaterialInfo::MT_Water)
			{
				FrameWaterSurfaces[(*it).first] = (*it).second.second;
				(*it).second.second.resize(0);
				continue;
			}

			info->Constantbuffer->BindToPixelShader(2);
			
		
			if((*it).first->GetSurface() && (*it).first->GetSurface()->GetEngineTexture())
			{
				//(*it).first->GetSurface()->GetEngineTexture()->BindToPixelShader(0);
			}else
			{
				//(*it).first->CacheIn(0.6f);
			}
			// Bind texture
			if((*it).first->CacheIn(0.6f) == zRES_CACHED_IN)
				(*it).first->Bind(0);
			else
				continue;

			// TODO: TESTCODE
			/*if(strnicmp((*it).first.Material->GetTexture()->GetNameWithoutExt().c_str(), "NW_Harbour_Stairs", strlen("NW_Harbour_Stairs")) == 0)
			{
			SetActiveVertexShader("VS_ExDisplace");
			ActiveVS->Apply();
			}else
			{
			SetActiveVertexShader("VS_Ex");
			ActiveVS->Apply();
			}*/

			// Querry the second texture slot to see if there is a normalmap bound
			ID3D11ShaderResourceView* nrmmap;
			Context->PSGetShaderResources(1,1, &nrmmap);
			if(!nrmmap)
			{
				if(ActivePS != defaultPS)
				{
					ActivePS = defaultPS;
					ActivePS->Apply();
				}
			}else
			{
				if(ActivePS != nrmPS)
				{
					ActivePS = nrmPS;
					ActivePS->Apply();
				}
				nrmmap->Release();
			}

			// Check for overwrites (TODO: This is slow, sort this!)
			if(!info->VertexShader.empty())
			{
				SetActiveVertexShader(info->VertexShader);
				if(ActiveVS)ActiveVS->Apply();
			}else if(ActiveVS != vsEx)
			{
				ActiveVS = vsEx;
				ActiveVS->Apply();
			}

			if(!info->TesselationShaderPair.empty())
			{
				

				info->Constantbuffer->BindToDomainShader(2);

				Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

				D3D11HDShader* hd = ShaderManager->GetHDShader(info->TesselationShaderPair);
				if(hd)hd->Apply();

				ActiveHDS = hd;

				DefaultHullShaderConstantBuffer hscb;

				// convert to EdgesPerScreenHeight
				hscb.H_EdgesPerScreenHeight = GetResolution().y / Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
				hscb.H_Proj11 = Engine::GAPI->GetRendererState()->TransformState.TransformProj._22;
				hscb.H_GlobalTessFactor = Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
				hscb.H_ScreenResolution = float2(GetResolution().x, GetResolution().y);
				hscb.H_FarPlane = Engine::GAPI->GetFarPlane();
				hd->GetConstantBuffer()[0]->UpdateBuffer(&hscb);
				hd->GetConstantBuffer()[0]->BindToHullShader(1);
			

			}else if(ActiveHDS)
			{
				ActiveHDS = NULL;
			
				// Bind wrapped mesh vertex buffers
				DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);
				Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				D3D11HDShader::Unbind();
			}

			if(!info->PixelShader.empty())
			{
				SetActivePixelShader(info->PixelShader);
				if(ActivePS)ActivePS->Apply();

			}else if(ActivePS != defaultPS && ActivePS != nrmPS)
			{
				// Querry the second texture slot to see if there is a normalmap bound
				ID3D11ShaderResourceView* nrmmap;
				Context->PSGetShaderResources(1,1, &nrmmap);
				if(!nrmmap)
				{
					if(ActivePS != defaultPS)
					{
						ActivePS = defaultPS;
						ActivePS->Apply();
					}
				}else
				{
					if(ActivePS != nrmPS)
					{
						ActivePS = nrmPS;
						ActivePS->Apply();
					}
					nrmmap->Release();
				}
			}
		}
		
		if(ActiveHDS)
		{
			for(std::vector<WorldMeshInfo*>::iterator itr = (*it).second.second.begin(); itr != (*it).second.second.end(); itr++)
			{
				DrawVertexBufferIndexed((*itr)->MeshVertexBuffer, (*itr)->MeshIndexBuffer, (*itr)->Indices.size());
			}
		}else
		{
			for(std::vector<WorldMeshInfo*>::iterator itr = (*it).second.second.begin(); itr != (*it).second.second.end(); itr++)
			{
				// Draw from wrapped mesh
				DrawVertexBufferIndexedUINT(NULL, NULL, (*itr)->Indices.size(), (*itr)->BaseIndexLocation);			
			}
		}

		Engine::GAPI->GetRendererState()->RendererInfo.WorldMeshDrawCalls += (*it).second.second.size();

		// Clear the list, leaving the memory allocated
		(*it).second.second.resize(0);
	}

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeWorld)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
	}

	Engine::GAPI->GetRendererState()->RendererInfo.FrameNumSectionsDrawn = numSections;
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	return XR_SUCCESS;
}

/** Draws the given mesh infos as water */
void D3D11GraphicsEngine::DrawWaterSurfaces()
{
	SetDefaultStates();

	// Copy backbuffer
	PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), PfxRenderer->GetTempBuffer()->GetRenderTargetView());
	CopyDepthStencil();

	// Pre-Draw the surfaces to fix overlaying polygons causing a huge performance drop
	// Unbind pixelshader
	Context->PSSetShader(NULL, NULL, NULL);
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());
	for(std::hash_map<zCTexture*, std::vector<WorldMeshInfo*>>::const_iterator it = FrameWaterSurfaces.begin(); it != FrameWaterSurfaces.end();it++)
	{
		// Draw surfaces
		for(unsigned int i=0;i<(*it).second.size();i++)
		{
			DrawVertexBufferIndexed((*it).second[i]->MeshVertexBuffer, (*it).second[i]->MeshIndexBuffer, (*it).second[i]->Indices.size());
		}
	}

	// Setup depth state so we can't have multiple layers of water
	Engine::GAPI->GetRendererState()->DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_LESS;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view); // Update view transform

	// Bind water shader
	SetActiveVertexShader("VS_ExWater");
	SetActivePixelShader("PS_Water");
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();



	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	// Bind distortion texture
	DistortionTexture->BindToPixelShader(4);

	// Bind copied backbuffer
	Context->PSSetShaderResources(5, 1, PfxRenderer->GetTempBuffer()->GetShaderResViewPtr());

	// Unbind depth buffer
	//Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), NULL);

	// Bind depth to the shader
	DepthStencilBufferCopy->BindToPixelShader(Context, 2);

	// Fill refraction info CB and bind it
	RefractionInfoConstantBuffer ricb;
	ricb.RI_Projection = Engine::GAPI->GetProjectionMatrix();
	ricb.RI_ViewportSize = float2(Resolution.x, Resolution.y);
	ricb.RI_Time = Engine::GAPI->GetTimeSeconds();
	ricb.RI_CameraPosition = Engine::GAPI->GetCameraPosition();

	ActivePS->GetConstantBuffer()[2]->UpdateBuffer(&ricb);
	ActivePS->GetConstantBuffer()[2]->BindToPixelShader(2);

	// Bind reflection cube
	Context->PSSetShaderResources(3,1, &ReflectionCube);



	for(int i=0;i<1;i++) // Draw twice, but second time only to depth buffer to fix the fog
	{
		for(std::hash_map<zCTexture*, std::vector<WorldMeshInfo*>>::const_iterator it = FrameWaterSurfaces.begin(); it != FrameWaterSurfaces.end();it++)
		{
			if((*it).first)
			{
				// Bind diffuse
				if((*it).first->CacheIn(-1) == zRES_CACHED_IN) // Force immediate cache in, because water is important!
					(*it).first->Bind(0);
			}

			// Draw surfaces
			for(unsigned int i=0;i<(*it).second.size();i++)
			{
				DrawVertexBufferIndexed((*it).second[i]->MeshVertexBuffer, (*it).second[i]->MeshIndexBuffer, (*it).second[i]->Indices.size());
			}
		}
	}

	// Draw Ocean
	Engine::GAPI->GetOcean()->Draw();

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	Engine::GAPI->GetRendererState()->DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_LESS_EQUAL;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
}

/** Draws everything around the given position */
void D3D11GraphicsEngine::DrawWorldAround(const D3DXVECTOR3& position, int sectionRange, float vobXZRange)
{
	// Setup renderstates
	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	Engine::GAPI->ResetWorldTransform();
	Engine::GAPI->SetViewTransform(view);

	// Set shader
	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_World");
	D3D11PShader* defaultPS = ActivePS;
	SetActiveVertexShader("VS_Ex");

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	D3DXMATRIX id;
	D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	// Update and bind buffer of PS
	PerObjectState ocb;
	ocb.OS_AmbientColor = float3(1,1,1);
	ActivePS->GetConstantBuffer()[3]->UpdateBuffer(&ocb);
	ActivePS->GetConstantBuffer()[3]->BindToPixelShader(3);

	INT2 s = WorldConverter::GetSectionOfPos(position);
	D3DXVECTOR2 camXZ = D3DXVECTOR2(position.x, position.z);

	float vobOutdoorDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius;
	float vobOutdoorSmallDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius;
	float vobSmallSize = Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize;

	DistortionTexture->BindToPixelShader(0);

	// Unbind PS
	//Context->PSSetShader(NULL, NULL, NULL);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled = false;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	UpdateRenderStates();

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawWorldMesh)
	{
		// Bind wrapped mesh vertex buffers
		DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, 0, 0);

		D3DXMATRIX id;
		D3DXMatrixIdentity(&id);
		ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&id);
		ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

		for(std::map<int, std::map<int, WorldMeshSectionInfo>>::iterator itx = Engine::GAPI->GetWorldSections().begin(); itx != Engine::GAPI->GetWorldSections().end(); itx++)
		{
			for(std::map<int, WorldMeshSectionInfo>::iterator ity = (*itx).second.begin(); ity != (*itx).second.end(); ity++)
			{
				D3DXVECTOR2 a = D3DXVECTOR2((float)((*itx).first - s.x), (float)((*ity).first - s.y));
				if(D3DXVec2Length(&a) < sectionRange)
				{
					WorldMeshSectionInfo& section = (*ity).second;

					if(Engine::GAPI->GetRendererState()->RendererSettings.FastShadows)
					{
						// Draw world mesh
						if(section.FullStaticMesh)
							Engine::GAPI->DrawMeshInfo(NULL, section.FullStaticMesh);
					}else
					{
						for(std::map<MeshKey, WorldMeshInfo*>::iterator it = section.WorldMeshes.begin(); it != section.WorldMeshes.end();it++)
						{
							// Check surface type
							if((*it).first.Info->MaterialType == MaterialInfo::MT_Water)
							{
								continue;
							}

							// Bind texture			
							if((*it).first.Material && (*it).first.Material->GetTexture())
							{
								if((*it).first.Material->GetTexture()->HasAlphaChannel())
								{
									if((*it).first.Material->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
									{
										(*it).first.Material->GetTexture()->Bind(0);
										ActivePS->Apply();
									}else
										continue; // Don't render if not loaded
								}else
								{
									// Unbind PS
									Context->PSSetShader(NULL, NULL, NULL);
								}
							}

							// Draw from wrapped mesh
							DrawVertexBufferIndexedUINT(Engine::GAPI->GetWrappedWorldMesh()->MeshVertexBuffer, Engine::GAPI->GetWrappedWorldMesh()->MeshIndexBuffer, (*it).second->Indices.size(), (*it).second->BaseIndexLocation);
						
							//Engine::GAPI->DrawMeshInfo((*it).first.Material, (*it).second);
						}

						// Draw all vobs here
						/*for(std::list<VobInfo*>::iterator it = section.Vobs.begin(); it != section.Vobs.end(); it++)
						{
							D3DXVECTOR2 xz = D3DXVECTOR2((*it)->LastRenderPosition.x, (*it)->LastRenderPosition.z);

							if(!(*it)->VisualInfo)
								continue; // Seems to happen in Gothic 1

							// Check vob range
							float range = D3DXVec2Length(&(camXZ - xz));
							if(range > vobOutdoorDist || ((*it)->VisualInfo->MeshSize < vobSmallSize && range > vobOutdoorSmallDist))
								continue;

							// Check for inside vob
							if((*it)->IsIndoorVob)
								continue;

							// Bind per-instance buffer
							((D3D11ConstantBuffer *)(*it)->VobConstantBuffer)->BindToVertexShader(1);

							// Draw the vob
							for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator itm = (*it)->VisualInfo->Meshes.begin(); itm != (*it)->VisualInfo->Meshes.end();itm++)
							{
								if((*itm).first && (*itm).first->GetTexture())
								{
									if((*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_NONE || 
										(*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_MAT_DEFAULT)
									{
										if((*itm).first->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
										{
											(*itm).first->GetTexture()->Bind(0);
										}
									}else
									{
										DistortionTexture->BindToPixelShader(0);
									}
								}

								for(unsigned int i=0;i<(*itm).second.size();i++)
								{
									Engine::GraphicsEngine->DrawVertexBufferIndexed((*itm).second[i]->MeshVertexBuffer, (*itm).second[i]->MeshIndexBuffer, (*itm).second[i]->Indices.size());
								}
							}
						}*/
					}
				}
			}
		}
	}

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
	{
		// Draw visible vobs here
		/*for(std::list<VobInfo*>::iterator it = RenderedVobs.begin(); it != RenderedVobs.end(); it++)
		{
			//D3DXVECTOR2 xz = D3DXVECTOR2((*it)->LastRenderPosition.x, (*it)->LastRenderPosition.z);

			if(!(*it)->VisualInfo)
				continue; // Seems to happen in Gothic 1

			// Check vob range
			float range = D3DXVec3Length(&(position - (*it)->LastRenderPosition));
			if(range > vobOutdoorDist || ((*it)->VisualInfo->MeshSize < vobSmallSize && range > vobOutdoorSmallDist))
				continue;

			// Check for inside vob
			if((*it)->IsIndoorVob)
				continue;

			// Bind per-instance buffer
			((D3D11ConstantBuffer *)(*it)->VobConstantBuffer)->BindToVertexShader(1);

			// Draw the vob
			for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator itm = (*it)->VisualInfo->Meshes.begin(); itm != (*it)->VisualInfo->Meshes.end();itm++)
			{
				if((*itm).first && (*itm).first->GetTexture())
				{
					if((*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_NONE || 
						(*itm).first->GetAlphaFunc() != zMAT_ALPHA_FUNC_FUNC_MAT_DEFAULT)
					{
						if((*itm).first->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
						{
							(*itm).first->GetTexture()->Bind(0);
						}
					}else
					{
						DistortionTexture->BindToPixelShader(0);
					}
				}

				for(unsigned int i=0;i<(*itm).second.size();i++)
				{
					Engine::GraphicsEngine->DrawVertexBufferIndexed((*itm).second[i]->MeshVertexBuffer, (*itm).second[i]->MeshIndexBuffer, (*itm).second[i]->Indices.size());
				}
				}
				}*/

		// Vobs have this differently
		Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = !Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise;
		Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
		UpdateRenderStates();

		// Reset instances
		const std::hash_map<zCProgMeshProto*, MeshVisualInfo*>& vis = Engine::GAPI->GetStaticMeshVisuals();
		/*for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
		{
		(*it).second->StartNewFrame();
		}*/

		D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
		float shadowRange = Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale * WorldShadowmap1->GetSizeX();
		for(std::list<VobInfo*>::iterator it = RenderedVobs.begin(); it != RenderedVobs.end(); it++)
		{
			VobInstanceInfo vii;
			vii.world = (*it)->WorldMatrix;

			if(!(*it)->IsIndoorVob)// && D3DXVec3Length(&((*it)->LastRenderPosition - position)) < shadowRange)
				((MeshVisualInfo *)(*it)->VisualInfo)->Instances.push_back(vii);
		}

		// Apply instancing shader
		SetActiveVertexShader("VS_ExInstancedObj");
		SetActivePixelShader("PS_DiffuseAlphaTest");
		ActiveVS->Apply();
		Context->PSSetShader(NULL, NULL, NULL);

		D3D11_BUFFER_DESC desc;
		((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()->GetDesc(&desc);

		/*if(desc.ByteWidth < sizeof(VobInstanceInfo) * vobs.size())
		{
			LogInfo() << "Instancing buffer too small (" << desc.ByteWidth << "), need " << sizeof(VobInstanceInfo) * vobs.size() << " bytes. Recreating buffer.";

			// Buffer too small, recreate it
			delete DynamicInstancingBuffer;
			DynamicInstancingBuffer = new D3D11VertexBuffer();
			DynamicInstancingBuffer->Init(NULL, sizeof(VobInstanceInfo) * vobs.size(), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
		}*/


		// Update the vertexbuffer
		//DynamicInstancingBuffer->UpdateBufferAligned16(&s_InstanceData[0], sizeof(VobInstanceInfo) * s_InstanceData.size());

		byte* data;
		UINT size;
		UINT loc = 0;
		DynamicInstancingBuffer->Map(BaseVertexBuffer::M_WRITE_DISCARD, (void**)&data, &size);
		static std::vector<VobInstanceInfo, AlignmentAllocator<VobInstanceInfo, 16> > s_InstanceData;
		for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
		{
			if((*it).second->Instances.empty())
				continue;

			if((loc + (*it).second->Instances.size()) * sizeof(VobInstanceInfo) >= desc.ByteWidth)
				break; // Should never happen

			(*it).second->StartInstanceNum = loc;
			memcpy(data + loc * sizeof(VobInstanceInfo), &(*it).second->Instances[0], sizeof(VobInstanceInfo) * (*it).second->Instances.size());
			loc += (*it).second->Instances.size();
		}
		DynamicInstancingBuffer->Unmap();

		{


			// Draw all vobs the player currently sees
			for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
			{
				if((*it).second->Instances.empty())
					continue;

				bool doReset = true;
				for(std::map<MeshKey, std::vector<MeshInfo*>>::iterator itt = (*it).second->MeshesByTexture.begin(); itt != (*it).second->MeshesByTexture.end(); itt++)
				{
					std::vector<MeshInfo *>& mlist = (*it).second->MeshesByTexture[(*itt).first];
					if(mlist.empty())
						continue;

					for(unsigned int i=0;i<mlist.size();i++)
					{
						zCTexture* tx = (*itt).first.Texture;

						// Check for alphablend
						bool blendAdd = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD;
						bool blendBlend = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND;
						if(!doReset || blendAdd || blendBlend) // FIXME: if one part of the mesh uses blending, all do. 
						{
							doReset = false;
							continue;
						}

						// Bind texture
						if(tx && tx->HasAlphaChannel())
						{
							if(tx->CacheIn(0.6f) == zRES_CACHED_IN)
							{
								tx->Bind(0);
								ActivePS->Apply();
							}else
								continue;
						}else
						{
							// Unbind PS
							Context->PSSetShader(NULL, NULL, NULL);
						}

					
						MeshInfo* mi = mlist[i];

						// Draw batch
						DrawInstanced(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size(), DynamicInstancingBuffer, sizeof(VobInstanceInfo), (*it).second->Instances.size(), sizeof(ExVertexStruct), (*it).second->StartInstanceNum);

					

						Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnVobs += (*it).second->Instances.size();
						//Engine::GraphicsEngine->DrawVertexBufferIndexed(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size());
					}			
				}

				// Reset visual
				if(doReset)
					(*it).second->StartNewFrame();
			}
		}
	}

	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawSkeletalMeshes)
	{
		// Draw skeletal meshes
		for(std::list<SkeletalVobInfo *>::iterator it = Engine::GAPI->GetSkeletalMeshVobs().begin(); it != Engine::GAPI->GetSkeletalMeshVobs().end(); it++)
		{
			//Engine::GraphicsEngine->GetLineRenderer()->AddAABB((*it)->Vob->GetPositionWorld(), D3DXVECTOR3(10,10,10));

			if(!(*it)->VisualInfo)
				continue;

			//if((*it)->VisualInfo->Meshes.empty() && (*it)->VisualInfo->NodeAttachments.empty())
			//	continue;

			INT2 s = WorldConverter::GetSectionOfPos((*it)->Vob->GetPositionWorld());

			float dist = D3DXVec3Length(&((*it)->Vob->GetPositionWorld() - position));
			if(dist > Engine::GAPI->GetRendererState()->RendererSettings.IndoorVobDrawRadius)
				continue; // Skip out of range

			Engine::GAPI->DrawSkeletalMeshVob((*it), FLT_MAX);
		}
	}

	Engine::GAPI->GetRendererState()->BlendState.ColorWritesEnabled = true;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
}

/** Draws the static vobs instanced */
XRESULT D3D11GraphicsEngine::DrawVOBsInstanced()
{
	START_TIMING();

	const std::hash_map<zCProgMeshProto*, MeshVisualInfo*>& vis = Engine::GAPI->GetStaticMeshVisuals();

	SetDefaultStates();

	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_Diffuse");
	D3D11PShader* defaultPS = ActivePS;
	SetActiveVertexShader("VS_ExInstancedObj");

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	// Use default material info for now
	MaterialInfo defInfo;
	ActivePS->GetConstantBuffer()[2]->UpdateBuffer(&defInfo);
	ActivePS->GetConstantBuffer()[2]->BindToPixelShader(2);

	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	INT2 camSection = WorldConverter::GetSectionOfPos(camPos);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
	}

	// Vobs need this
	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = true;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	static std::vector<VobInfo *> vobs;
	static std::vector<VobLightInfo *> lights;
	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs || 
		Engine::GAPI->GetRendererState()->RendererSettings.EnableDynamicLighting)
	{
		if(!Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum ||
			(Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum && vobs.empty()))
			Engine::GAPI->CollectVisibleVobs(vobs, lights);
	}

	// Need to collect alpha-meshes to render them laterdy
	std::list<std::pair<MeshKey, std::pair<MeshVisualInfo*, MeshInfo*>>> AlphaMeshes;
	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
	{
		static std::vector<VobInstanceInfo, AlignmentAllocator<VobInstanceInfo, 16> > s_InstanceData;
		/*for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
		{
#ifdef BUILD_GOTHIC_1_08k // G1 has this sometimes?
			if(!(*it).second->Visual)
				continue;
#endif

			(*it).second->StartInstanceNum = s_InstanceData.size();
			s_InstanceData.insert(s_InstanceData.end(), (*it).second->Instances.begin(), (*it).second->Instances.end());
		}
		*/
			//if(!s_InstanceData.empty())
			{

			/*if(s_InstanceData.size() * sizeof(VobInstanceInfo) % 16 != 0)
			{
				int d = 16 - (s_InstanceData.size() * sizeof(VobInstanceInfo) % 16); // Calculate missing bytes
				int numToAdd = d / sizeof(VobInstanceInfo); // Amount of instances we have to add (max 7)


				VobInstanceInfo zi;
				memset(&zi, 0, sizeof(zi));

				// Add missing data to make it aligned
				s_InstanceData.resize(s_InstanceData.size() + numToAdd, zi);
			}*/

			// Create instancebuffer for this frame
			D3D11_BUFFER_DESC desc;
			((D3D11VertexBuffer *)DynamicInstancingBuffer)->GetVertexBuffer()->GetDesc(&desc);

			if(desc.ByteWidth < sizeof(VobInstanceInfo) * vobs.size())
			{
				LogInfo() << "Instancing buffer too small (" << desc.ByteWidth << "), need " << sizeof(VobInstanceInfo) * vobs.size() << " bytes. Recreating buffer.";

				// Buffer too small, recreate it
				delete DynamicInstancingBuffer;
				DynamicInstancingBuffer = new D3D11VertexBuffer();
				DynamicInstancingBuffer->Init(NULL, sizeof(VobInstanceInfo) * vobs.size(), BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
			}

			
			// Update the vertexbuffer
			//DynamicInstancingBuffer->UpdateBufferAligned16(&s_InstanceData[0], sizeof(VobInstanceInfo) * s_InstanceData.size());

			byte* data;
			UINT size;
			UINT loc = 0;
			DynamicInstancingBuffer->Map(BaseVertexBuffer::M_WRITE_DISCARD, (void**)&data, &size);
				static std::vector<VobInstanceInfo, AlignmentAllocator<VobInstanceInfo, 16> > s_InstanceData;
				for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
				{
					(*it).second->StartInstanceNum = loc;
					memcpy(data + loc * sizeof(VobInstanceInfo), &(*it).second->Instances[0], sizeof(VobInstanceInfo) * (*it).second->Instances.size());
					loc += (*it).second->Instances.size();
				}
			DynamicInstancingBuffer->Unmap();

			for(unsigned int i=0;i<vobs.size();i++)
			{
				vobs[i]->VisibleInRenderPass = false; // Reset this for the next frame
				RenderedVobs.push_back(vobs[i]);
			}

			// Reset buffer
			s_InstanceData.resize(0);

			for(std::hash_map<zCProgMeshProto*, MeshVisualInfo*>::const_iterator it = vis.begin(); it != vis.end(); it++)
			{
				if((*it).second->Instances.empty())
					continue;

				if((*it).second->MeshSize < Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize)
				{
					OutdoorSmallVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius - (*it).second->MeshSize, 0, 0, 0));
					OutdoorSmallVobsConstantBuffer->BindToPixelShader(3);
				}
				else
				{
					OutdoorVobsConstantBuffer->UpdateBuffer(&D3DXVECTOR4(Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius - (*it).second->MeshSize, 0, 0, 0));
					OutdoorVobsConstantBuffer->BindToPixelShader(3);
				}

				bool doReset = true; // Don't reset alpha-vobs here
				for(std::map<MeshKey, std::vector<MeshInfo*>>::iterator itt = (*it).second->MeshesByTexture.begin(); itt != (*it).second->MeshesByTexture.end(); itt++)
				{
					std::vector<MeshInfo *>& mlist = (*it).second->MeshesByTexture[(*itt).first];
					if(mlist.empty())
						continue;

					// Bind buffers
					DrawVertexBufferIndexed((*it).second->FullMesh->MeshVertexBuffer, (*it).second->FullMesh->MeshIndexBuffer, 0);

					for(unsigned int i=0;i<mlist.size();i++)
					{
						zCTexture* tx = (*itt).first.Texture;

						if(!tx)
							continue;

						// Check for alphablending on world mesh
						bool blendAdd = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD;
						bool blendBlend = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND;
						if(!doReset || blendAdd || blendBlend) // FIXME: if one part of the mesh uses blending, all do. 
						{
							MeshVisualInfo* info = (*it).second;
							MeshInfo* mesh = mlist[i];

							AlphaMeshes.push_back(std::make_pair((*itt).first, std::make_pair(info, mesh)));

							doReset = false;
							continue;
						}

						// Bind texture

						MeshInfo* mi = mlist[i];

						if(tx->CacheIn(0.6f) == zRES_CACHED_IN)
						{
							MyDirectDrawSurface7* surface = tx->GetSurface();
							ID3D11ShaderResourceView* srv[3];
			
							// Get diffuse and normalmap
							srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
							srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
							srv[2] = surface->GetFxMap() ? ((D3D11Texture *)surface->GetFxMap())->GetShaderResourceView() : NULL;

							// Bind both
							Context->PSSetShaderResources(0,3, srv);

							// Set normal/displacement map
							Context->DSSetShaderResources(0,1, &srv[1]);
							Context->HSSetShaderResources(0,1, &srv[1]);

							

							// Force alphatest on vobs for now
							BindShaderForTexture(tx, true, 0);
									
							MaterialInfo* info = (*itt).first.Info;
							if(!info->Constantbuffer)
								info->UpdateConstantbuffer();

							info->Constantbuffer->BindToPixelShader(2);

							

						}
						else
						{
#ifndef PUBLIC_RELEASE
							for(int s=0;s<(*it).second->Instances.size();s++)
							{
								D3DXVECTOR3 pos = D3DXVECTOR3((*it).second->Instances[s].world._14, (*it).second->Instances[s].world._24, (*it).second->Instances[s].world._34); 
								GetLineRenderer()->AddAABBMinMax(pos - (*it).second->BBox.Min, pos + (*it).second->BBox.Max, D3DXVECTOR4(1,0,0,1));
							}
#endif
							continue;
						}

						

						if(!mi->IndicesPNAEN.empty() && RenderingStage == DES_MAIN && (*it).second->TesselationInfo.buffer.VT_TesselationFactor > 0.0f)
						{
							
							Setup_PNAEN(PNAEN_Instanced);
							(*it).second->TesselationInfo.Constantbuffer->BindToDomainShader(1);
							(*it).second->TesselationInfo.Constantbuffer->BindToHullShader(1);
						}else if(ActiveHDS)
						{
							Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
							Context->DSSetShader(NULL, NULL, NULL);
							Context->HSSetShader(NULL, NULL, NULL);
							ActiveHDS = NULL;
							SetActiveVertexShader("VS_ExInstancedObj");
							ActiveVS->Apply();
						}

						if(!ActiveHDS)
						{
							// Draw batch
							DrawInstanced(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size(), DynamicInstancingBuffer, sizeof(VobInstanceInfo), (*it).second->Instances.size(), sizeof(ExVertexStruct), (*it).second->StartInstanceNum);
						}else
						{
							// Draw batch tesselated
							DrawInstanced(mi->MeshVertexBuffer, mi->MeshIndexBufferPNAEN, mi->IndicesPNAEN.size(), DynamicInstancingBuffer, sizeof(VobInstanceInfo), (*it).second->Instances.size(), sizeof(ExVertexStruct), (*it).second->StartInstanceNum);
						}
						
						
						//DrawVertexBufferIndexed((*it).second->FullMesh->MeshVertexBuffer, (*it).second->FullMesh->MeshIndexBuffer, 0);

						//Engine::GraphicsEngine->DrawVertexBufferIndexed(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size());
					}

				}

				// Reset visual
				if(doReset && !Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum)
				{
					(*it).second->StartNewFrame();
				}
			}
		}
	}

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Context->DSSetShader(NULL, NULL, NULL);
	Context->HSSetShader(NULL, NULL, NULL);
	ActiveHDS = NULL;

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
	}

	STOP_TIMING(GothicRendererTiming::TT_Vobs);

	
	if(RenderingStage == DES_MAIN)
	{

		if(Engine::GAPI->GetRendererState()->RendererSettings.DrawParticleEffects)
		{
			std::vector<zCVob *> decals;
			Engine::GAPI->GetVisibleDecalList(decals);
			DrawDecalList(decals, true);

			DrawQuadMarks();
		}

		START_TIMING();
		// Draw lighting, since everything is drawn by now and we have the lights here
		DrawLighting(lights);

		STOP_TIMING(GothicRendererTiming::TT_Lighting);
	}

	// Make sure lighting doesn't mess up our state
	SetDefaultStates();

	SetActivePixelShader("PS_Simple");
	SetActiveVertexShader("VS_ExInstancedObj");

	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = true;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	for(auto itt = AlphaMeshes.begin(); itt != AlphaMeshes.end(); itt++)
	{
		zCTexture* tx = (*itt).first.Texture;

		if(!tx)
			continue;

		// Check for alphablending on world mesh
		bool blendAdd = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_ADD;
		bool blendBlend = (*itt).first.Material->GetAlphaFunc() == zMAT_ALPHA_FUNC_BLEND;

		// Bind texture

		MeshInfo* mi = (*itt).second.second;
		MeshVisualInfo* vi = (*itt).second.first;

		if(tx->CacheIn(0.6f) == zRES_CACHED_IN)
		{
			MyDirectDrawSurface7* surface = tx->GetSurface();
			ID3D11ShaderResourceView* srv[3];

			// Get diffuse and normalmap
			srv[0] = ((D3D11Texture *)surface->GetEngineTexture())->GetShaderResourceView();
			srv[1] = surface->GetNormalmap() ? ((D3D11Texture *)surface->GetNormalmap())->GetShaderResourceView() : NULL;
			srv[2] = surface->GetFxMap() ? ((D3D11Texture *)surface->GetFxMap())->GetShaderResourceView() : NULL;

			// Bind both
			Context->PSSetShaderResources(0,3, srv);

			if((blendAdd || blendBlend) && !Engine::GAPI->GetRendererState()->BlendState.BlendEnabled)
			{
				if(blendAdd)
					Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
				else if(blendBlend)
					Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();

				Engine::GAPI->GetRendererState()->BlendState.SetDirty();

				Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();

				UpdateRenderStates();
			}

			MaterialInfo* info = (*itt).first.Info;
			if(!info->Constantbuffer)
				info->UpdateConstantbuffer();

			info->Constantbuffer->BindToPixelShader(2);
		}

		// Draw batch
		DrawInstanced(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size(), DynamicInstancingBuffer, sizeof(VobInstanceInfo), vi->Instances.size(), sizeof(ExVertexStruct), vi->StartInstanceNum);


	}

	// Loop again, now that all alpha-meshes have been rendered
	// so we can reset their visuals, too.
	for(auto itt = AlphaMeshes.begin(); itt != AlphaMeshes.end(); itt++)
	{
		MeshVisualInfo* vi = (*itt).second.first;

		// Reset visual		
		vi->StartNewFrame();	
	}

	if(!Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum)
	{
		lights.resize(0);
		vobs.resize(0);
	}

	SetDefaultStates();

	return XR_SUCCESS;
}

/** Draws the static VOBs */
XRESULT D3D11GraphicsEngine::DrawVOBs(bool noTextures)
{
	return DrawVOBsInstanced();

	//Engine::GAPI->SetFarPlane(DEFAULT_FAR_PLANE);

	// bind shader
	SetActivePixelShader("PS_AtmosphereGround");
	D3D11PShader* nrmPS = ActivePS;
	SetActivePixelShader("PS_World");
	D3D11PShader* defaultPS = ActivePS;
	SetActiveVertexShader("VS_Ex");

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	// Use default material info for now
	MaterialInfo defInfo;
	ActivePS->GetConstantBuffer()[2]->UpdateBuffer(&defInfo);
	ActivePS->GetConstantBuffer()[2]->BindToPixelShader(2);

	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	INT2 camSection = WorldConverter::GetSectionOfPos(camPos);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	
	

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
	}

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	//DrawVOBsInstanced();

	// Static so they keep their capacity
	static std::vector<VobInfo *> vobs;
	static std::vector<VobLightInfo *> lights;

	if(!Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum)
		Engine::GAPI->CollectVisibleVobs(vobs, lights);

	//srand(0); // TODO: TESTCODE

	
	if(Engine::GAPI->GetRendererState()->RendererSettings.DrawVOBs)
	{
		float vobIndoorDist = Engine::GAPI->GetRendererState()->RendererSettings.IndoorVobDrawRadius;
		float vobOutdoorDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorVobDrawRadius;
		float vobOutdoorSmallDist = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius;
		float vobSmallSize = Engine::GAPI->GetRendererState()->RendererSettings.SmallVobSize;
		int sectionViewDist = Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius;


		//noTextures = true;
		float3 indoorAmbient = DEFAULT_INDOOR_VOB_AMBIENT;

		// Sort them per material
		std::hash_map<zCMaterial*, std::vector<VobInfo*>> meshesPerMaterial;

		for(std::vector<VobInfo*>::iterator itv = vobs.begin(); itv != vobs.end();itv++)
		{
			// Reset this vob for CollectVisibleVobs
			(*itv)->VisibleInRenderPass = false;

			if(!(*itv)->Vob->GetVisual() || !(*itv)->VisualInfo) // No visual info happens in G1 sometimes? Maybe I delete it and Smartheap NULLs it or something?
				continue;

			if(((MeshVisualInfo *)(*itv)->VisualInfo)->UnloadedSomething)
				continue; // Skip the whole mesh if we're missing something

			// Check for repositioning
			if(memcmp(&(*itv)->LastRenderPosition, (*itv)->Vob->GetPositionWorld(), sizeof(D3DXVECTOR3)) != 0)
			{
				(*itv)->LastRenderPosition = (*itv)->Vob->GetPositionWorld();
				(*itv)->UpdateVobConstantBuffer();

				Engine::GAPI->GetRendererState()->RendererInfo.FrameVobUpdates++;
			}

			//GetLineRenderer()->AddPointLocator((*itv)->LastRenderPosition + D3DXVECTOR3(1,1,1) * Toolbox::frand(), 20.0f, D3DXVECTOR4(Toolbox::frand(), Toolbox::frand(), Toolbox::frand(), 1));

			for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator itm = (*itv)->VisualInfo->Meshes.begin(); itm != (*itv)->VisualInfo->Meshes.end();itm++)
			{
				meshesPerMaterial[(*itm).first].push_back((*itv));
			}

			Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnVobs++;
		}

		std::hash_map<zCTexture*, std::vector<VobInfo*>*> meshesPerTexture;
		for(std::hash_map<zCMaterial*, std::vector<VobInfo*>>::iterator itm = meshesPerMaterial.begin(); itm != meshesPerMaterial.end();itm++)
		{
			meshesPerTexture[(*itm).first->GetTexture()] = &(*itm).second;
		}

		struct VobVisualCmpStruct
		{
			static bool cmpVisuals(const VobInfo* a, const VobInfo* b)
			{
				return a->VisualInfo < b->VisualInfo;
			}
		};

		for(std::hash_map<zCTexture*, std::vector<VobInfo*>*>::iterator itm = meshesPerTexture.begin(); itm != meshesPerTexture.end();itm++)
		{
			// Bind texture
			if((*itm).first)
			{
				if((*itm).first->CacheIn(0.6f) == zRES_CACHED_IN)
					(*itm).first->Bind(0);
				else
					continue;
			}

			// Querry the second texture slot to see if there is a normalmap bound
			ID3D11ShaderResourceView* nrmmap;
			Context->PSGetShaderResources(1,1, &nrmmap);
			if(!nrmmap)
			{
				if(ActivePS != defaultPS)
				{
					ActivePS = defaultPS;
					ActivePS->Apply();
				}
			}else
			{
				MaterialInfo* info = Engine::GAPI->GetMaterialInfoFrom((*itm).first);
				if(!info->Constantbuffer)
					info->UpdateConstantbuffer();

				info->Constantbuffer->BindToPixelShader(2);

				if(ActivePS != nrmPS)
				{
					ActivePS = nrmPS;
					ActivePS->Apply();
				}
				nrmmap->Release();
			}

			// Sort the meshes for using the same visual
			//(*itm).second->sort(VobVisualCmpStruct::cmpVisuals);
			MeshVisualInfo* activeVisual = NULL;

			// Draw everything which uses this material
			for(std::vector<VobInfo*>::iterator itv = (*itm).second->begin(); itv != (*itm).second->end();itv++)
			{
				// Bind per-instance buffer
				((D3D11ConstantBuffer *)(*itv)->VobConstantBuffer)->BindToVertexShader(1);

				// Update and bind buffer of PS
				/*PerObjectState ocb;
				if((*itv)->IsIndoorVob)
				{
					ocb.OS_AmbientColor = indoorAmbient;
				}else
				{
					ocb.OS_AmbientColor = float3(1,1,1);
				}
				ActivePS->GetConstantBuffer()[3]->UpdateBuffer(&ocb);
				ActivePS->GetConstantBuffer()[3]->BindToPixelShader(3);*/

				
				MeshKey key;
				key.Texture = (*itm).first;
				std::vector<MeshInfo *>& mlist = ((MeshVisualInfo *)(*itv)->VisualInfo)->MeshesByTexture[key];
				if(mlist.empty())
					continue;

				/*MeshInfo* mi = mlist[0]; // They should only have this once

				if(activeVisual == (*itv)->VisualInfo)
				{
					// This is not the first instance of this visual we are drawing, use the already bound buffers
					Engine::GraphicsEngine->DrawVertexBufferIndexed(NULL, NULL, mi->Indices.size());
				}else
				{
					// Update bound vb and set this as active
					Engine::GraphicsEngine->DrawVertexBufferIndexed(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size());
					activeVisual = (*itv)->VisualInfo;
				}*/

				for(unsigned int i=0;i<mlist.size();i++)
				{
					MeshInfo* mi = mlist[i];

					Engine::GraphicsEngine->DrawVertexBufferIndexed(mi->MeshVertexBuffer, mi->MeshIndexBuffer, mi->Indices.size());
				}

				RenderedVobs.push_back((*itv));
			}
		}

		/*for(std::list<VobInfo*>::iterator itv = vobs.begin(); itv != vobs.end();itv++)
		{
			

			

			// Push the vob
			for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator itm = (*itv)->VisualInfo->Meshes.begin(); itm != (*itv)->VisualInfo->Meshes.end();itm++)
			{
				if(!noTextures)
				{
					
				}

				for(unsigned int i=0;i<(*itm).second.size();i++)
				{
					// Draw instances
					Engine::GraphicsEngine->DrawVertexBufferIndexed((*itm).second[i]->MeshVertexBuffer, (*itm).second[i]->MeshIndexBuffer, (*itm).second[i]->Indices.size());
				}
			}
		}*/
	}

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeVobs)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
	}

	if(RenderingStage == DES_MAIN)
	{
		// Draw lighting, since everything is drawn by now and we have the lights here
		DrawLighting(lights);
	}

	if(!Engine::GAPI->GetRendererState()->RendererSettings.FixViewFrustum)
	{
		// Clear vectors, but don't reallocate
		vobs.resize(0);
		lights.resize(0);
	}

	return XR_SUCCESS;
}


/** Returns the current size of the backbuffer */
INT2 D3D11GraphicsEngine::GetResolution()
{
	return Resolution;
}

/** Returns the actual resolution of the backbuffer (not supersampled) */
INT2 D3D11GraphicsEngine::GetBackbufferResolution()
{
	// Get desktop rect
	RECT desktop;
	GetClientRect(GetDesktopWindow(), &desktop);

	if(Resolution.x > desktop.right || Resolution.y > desktop.bottom)
		return INT2(desktop.right, desktop.bottom);

	return Resolution;
}

/** Sets up the default rendering state */
void D3D11GraphicsEngine::SetDefaultStates()
{
	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDefault();
	Engine::GAPI->GetRendererState()->SamplerState.SetDefault();

	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
	Engine::GAPI->GetRendererState()->SamplerState.SetDirty();

	Context->PSSetSamplers(0, 1, &DefaultSamplerState);

	UpdateRenderStates();
}

/** Draws the 3D-Cloud-Sky */
void D3D11GraphicsEngine::Draw3DClouds()
{
	GSky* sky = Engine::GAPI->GetSky();

	SetDefaultStates();

	// Copy the scene to the cloud-buffer without alpha
	SetActivePixelShader("PS_PFX_Copy_NoAlpha");
	ActivePS->Apply();

	Context->ClearRenderTargetView(CloudBuffer->GetRenderTargetView(), (float *)&float4(0,0,0,0));
	PfxRenderer->CopyTextureToRTV(BackbufferSRV, CloudBuffer->GetRenderTargetView(), INT2(0,0), true);

	Context->OMSetRenderTargets(1, CloudBuffer->GetRenderTargetViewPtr(), NULL);

	// Draw cloud meshes
	DrawCloudMeshes();

	Context->OMSetRenderTargets(1, &BackbufferRTV, DepthStencilBuffer->GetDepthStencilView());

	// Blur the cloud-buffer
	PfxRenderer->BlurTexture(CloudBuffer, true);

	// Blend it to the backbuffer
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	UpdateRenderStates();

	SetActivePixelShader("PS_SkyPlane");
	SetActiveVertexShader("VS_Ex");
	ActiveVS->Apply();
	ActivePS->Apply();

	ViewportInfoConstantBuffer vpi;
	vpi.VPI_ViewportSize = float2((float)CloudBuffer->GetSizeX(), (float)CloudBuffer->GetSizeY());
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&vpi);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	Context->OMSetRenderTargets(1, &BackbufferRTV, DepthStencilBuffer->GetDepthStencilView());

	PfxRenderer->GetTempBufferDS4_2()->BindToPixelShader(Context, 2);
	NoiseTexture->BindToPixelShader(1);
	CloudBuffer->BindToPixelShader(Context, 0);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetWorldTransform(view, true);

	Engine::GAPI->DrawMeshInfo(NULL, sky->GetSkyPlane());
	//PfxRenderer->CopyTextureToRTV(CloudBuffer->GetShaderResView(), BackbufferRTV, INT2(0,0), true);
}

/** Draws the sky using the GSky-Object */
XRESULT D3D11GraphicsEngine::DrawSky()
{
	GSky* sky = Engine::GAPI->GetSky();
	sky->RenderSky();

	//Context->OMSetRenderTargets(1, &BackbufferRTV, NULL);

	if(!Engine::GAPI->GetRendererState()->RendererSettings.AtmosphericScattering)
		return XR_SUCCESS;

	// Create a rotaion only view-matrix
	D3DXMATRIX invView;
	Engine::GAPI->GetInverseViewMatrix(&invView);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);



	// Remove translation
	/*invView(0,3) = 0;
	invView(1,3) = 0;
	invView(2,3) = 0;*/

	D3DXMATRIX scale;
	D3DXMatrixScaling(&scale, sky->GetAtmoshpereSettings().OuterRadius,
		sky->GetAtmoshpereSettings().OuterRadius,
		sky->GetAtmoshpereSettings().OuterRadius); // Upscale it a huge amount. Gothics world is big.

	D3DXMATRIX world;
	D3DXMatrixTranslation(&world, Engine::GAPI->GetCameraPosition().x, 
		Engine::GAPI->GetCameraPosition().y + sky->GetAtmoshpereSettings().SphereOffsetY, 
		Engine::GAPI->GetCameraPosition().z);
	 
	world = scale * world;
	D3DXMatrixTranspose(&world, &world);

	// Apply world matrix
	Engine::GAPI->SetWorldTransform(world);
	Engine::GAPI->SetViewTransform(view);

	if(sky->GetAtmosphereCB().AC_CameraHeight > sky->GetAtmosphereCB().AC_OuterRadius)
	{
		SetActivePixelShader("PS_AtmosphereOuter");
	}else
	{
		SetActivePixelShader("PS_Atmosphere");
	}

	
	SetActiveVertexShader("VS_ExWS");

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(1);

	VS_ExConstantBuffer_PerInstance cbi;
	cbi.World = world;
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cbi);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);


	// Update sky CB
	/*SkyConstantBuffer scb;
	scb.SC_TextureWeight = factor;
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&scb),
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);
	*/
	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;

	Engine::GAPI->GetRendererState()->DepthState.SetDefault();

	// Allow z-testing
	Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled = true;

	// Disable depth-writes so the sky always stays at max distance in the DepthBuffer
	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;

	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Apply sky texture
	BaseTexture* cloudsTex = Engine::GAPI->GetSky()->GetCloudTexture();
	if(cloudsTex)
	{
		cloudsTex->BindToPixelShader(0);
	}

	BaseTexture* nightTex = Engine::GAPI->GetSky()->GetNightTexture();
	if(nightTex)
	{
		nightTex->BindToPixelShader(1);
	}

	

	if(sky->GetSkyDome())
		sky->GetSkyDome()->DrawMesh();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = false;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	//Context->OMSetRenderTargets(1, &BackbufferRTV, DepthStencilBuffer->GetDepthStencilView());

	//SetActivePixelShader("PS_World");

	return XR_SUCCESS;
}

/** Renders the cloudmeshes */
void D3D11GraphicsEngine::DrawCloudMeshes()
{
	GSky* sky = Engine::GAPI->GetSky();

	// Always use the same seed for this
	const int SEED = 1000;
	srand(SEED);
	const int NUM_CLOUDS = 100;

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
	UpdateRenderStates();

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	SetActivePixelShader("PS_Cloud");
	SetActiveVertexShader("VS_Ex");
	ActivePS->Apply();
	ActiveVS->Apply();


	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	D3DXVECTOR3 tempSunPos = D3DXVECTOR3(0,20000,0);

	D3DXVECTOR3 minCloud = D3DXVECTOR3(-80000, 10000, -80000);
	D3DXVECTOR3 maxCloud = D3DXVECTOR3(80000, 10000, 80000);

	D3DXVECTOR3 minSize = D3DXVECTOR3(300, 300, 300);
	D3DXVECTOR3 maxSize = D3DXVECTOR3(1600, 600, 1600);

	for(int i=0;i<NUM_CLOUDS;i++)
	{
		D3DXVECTOR3 cp = D3DXVECTOR3(Toolbox::lerp(minCloud.x, maxCloud.x, Toolbox::frand()),
			Toolbox::lerp(minCloud.y, maxCloud.y, Toolbox::frand()),
			Toolbox::lerp(minCloud.z, maxCloud.z, Toolbox::frand()));

		D3DXVECTOR3 cs = D3DXVECTOR3(Toolbox::lerp(minSize.x, maxSize.x, Toolbox::frand()),
			Toolbox::lerp(minSize.y, maxSize.y, Toolbox::frand()),
			Toolbox::lerp(minSize.z, maxSize.z, Toolbox::frand()));

		CloudConstantBuffer ccb;
		D3DXVECTOR3 sp = cp - tempSunPos;
		D3DXVec3Normalize(&sp, &sp);

		D3DXVec3TransformNormal(ccb.C_LightDirection.toD3DXVECTOR3(), &sky->GetAtmoshpereSettings().LightDirection, &view);

		ccb.C_CloudPosition = cp;
		ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&ccb);
		ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

		D3DXMATRIX scale;
		D3DXMatrixScaling(&scale, cs.x, cs.y, cs.z); // Upscale

		D3DXMATRIX world;
		D3DXMatrixTranslation(&world, cp.x, cp.y, cp.z);
		world = scale * world;

		// Apply world matrix
		Engine::GAPI->SetWorldTransform(world);
		Engine::GAPI->SetViewTransform(view);

		// Draw
		sky->GetCloudMeshes()[rand() % 2]->DrawMesh();
	}

	srand(timeGetTime());
}

/** Returns the shadermanager */
D3D11ShaderManager* D3D11GraphicsEngine::GetShaderManager()
{
	return ShaderManager;
}

/** Called when a key got pressed */
XRESULT D3D11GraphicsEngine::OnKeyDown(unsigned int key)
{
	switch (key)
	{
	case VK_NUMPAD0:
		ReloadShaders();
		break;

	case VK_F1:		
		if(!UIView && !Engine::GAPI->GetRendererState()->RendererSettings.EnableEditorPanel)
		{
			// If the ui-view hasn't been created yet and the editorpanel is disabled, enable it here
			Engine::GAPI->GetRendererState()->RendererSettings.EnableEditorPanel = true;
			CreateMainUIView();
		}
		break;

	default:
		break;
	}

	return XR_SUCCESS;
}

/** Reloads shaders */
XRESULT D3D11GraphicsEngine::ReloadShaders()
{
	XRESULT xr = ShaderManager->ReloadShaders();

	return xr;
}

/** Returns the line renderer object */
BaseLineRenderer* D3D11GraphicsEngine::GetLineRenderer()
{
	return LineRenderer;
}

/** Applys the lighting to the scene */
XRESULT D3D11GraphicsEngine::DrawLighting(std::vector<VobLightInfo*>& lights)
{
	SetDefaultStates();

	// ********************************
	// Draw world shadows
	// ********************************
	CameraReplacement cr;

	// Get shadow direction, but don't update every frame, to get around flickering
	
	D3DXVECTOR3 dir = *Engine::GAPI->GetSky()->GetAtmosphereCB().AC_LightPos.toD3DXVECTOR3();
	static D3DXVECTOR3 oldDir = dir;
	static D3DXVECTOR3 smoothDir = dir;

	static D3DXVECTOR3 oldP = D3DXVECTOR3(0,0,0);
	D3DXVECTOR3 cameraPosition = Engine::GAPI->GetCameraPosition();
	D3DXVECTOR3 WorldShadowCP;

	// Update dir
	if(fabs(D3DXVec3Dot(&oldDir, &dir)) > 0.999f)
	{
		dir = oldDir;	
	}
	else
	{
		D3DXVECTOR3 target = dir;

		// Smoothly transition to the next state and wait there
		if(fabs(D3DXVec3Dot(&smoothDir, &dir)) < 0.9999f) // but cut it off somewhere or the pixels will flicker
			D3DXVec3Lerp(&dir, &smoothDir, &target, Engine::GAPI->GetFrameTimeSec() * 2.0f);
		else
			oldDir = dir;	

		smoothDir = dir;
	}

	float smPixelSize = WorldShadowmap1->GetSizeX() * Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale;


	// Update position
	if((D3DXVec3Length(&(oldP - cameraPosition)) < 64 && // Try to update only if the camera went 500 units away from the last position
		(cameraPosition.x - (int)cameraPosition.x) < 0.1f && // And is on even space
		(cameraPosition.y - (int)cameraPosition.y) < 0.1f) || 
		D3DXVec3Length(&(oldP - cameraPosition)) < 600.0f) // but don't let it go too far
	{
		WorldShadowCP = oldP;
	}else
	{
		oldP = cameraPosition;
		WorldShadowCP = cameraPosition;
	}



	// Get the section we are currently in
	INT2 cameraSection = WorldConverter::GetSectionOfPos(WorldShadowCP);
	WorldMeshSectionInfo& section = Engine::GAPI->GetWorldSections()[cameraSection.x][cameraSection.y];
	D3DXVECTOR3 p = WorldShadowCP;
	// Set the camera height to the highest point in this section
	//p.y = 0;
	p += dir * 6000.0f;

	D3DXVECTOR3 lookAt = p;
	lookAt -= dir;

	// Create shadowmap view-matrix
	D3DXMatrixLookAtLH(&cr.ViewReplacement, &p, &lookAt, &D3DXVECTOR3(0,1,0));
	D3DXMatrixTranspose(&cr.ViewReplacement, &cr.ViewReplacement);

	D3DXMatrixOrthoLH(&cr.ProjectionReplacement, WorldShadowmap1->GetSizeX() * Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale, 
												 WorldShadowmap1->GetSizeX() * Engine::GAPI->GetRendererState()->RendererSettings.WorldShadowRangeScale, 1, 20000.0f);
	D3DXMatrixTranspose(&cr.ProjectionReplacement, &cr.ProjectionReplacement);

	cr.PositionReplacement = p;
	cr.LookAtReplacement = lookAt;

	// Replace gothics camera
	Engine::GAPI->SetCameraReplacementPtr(&cr);

	if(Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetBspTreeMode() == zBSP_MODE_OUTDOOR) // Indoor worlds don't need shadowmaps for the world
	{
		RenderShadowmaps(WorldShadowCP);
	}

	SetDefaultStates();

	// Restore gothics camera
	Engine::GAPI->SetCameraReplacementPtr(NULL);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	// ********************************
	// Draw direct lighting
	// ********************************
	SetActiveVertexShader("VS_ExPointLight");
	SetActivePixelShader("PS_DS_PointLight");

	Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);

	Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	// Copy this, so we can access depth in the pixelshader and still use the buffer for culling
	CopyDepthStencil();

	// Set the main rendertarget
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());
	//Context->ClearRenderTargetView(HDRBackBuffer->GetRenderTargetView(), (float *)D3DXVECTOR4(0,0,0,0));

	
	D3DXMatrixTranspose(&view, &view);

	DS_PointLightConstantBuffer plcb;
	D3DXMatrixInverse(&plcb.PL_InvProj, NULL, &Engine::GAPI->GetProjectionMatrix());
	D3DXMatrixInverse(&plcb.PL_InvView, NULL, &Engine::GAPI->GetRendererState()->TransformState.TransformView);
	plcb.PL_ViewportSize = float2((float)Resolution.x, (float)Resolution.y);

	GBuffer0_Diffuse->BindToPixelShader(Context, 0);
	GBuffer1_Normals_SpecIntens_SpecPower->BindToPixelShader(Context, 1);
	DepthStencilBufferCopy->BindToPixelShader(Context, 2);

	bool lastOutside = true;

	// Draw all lights
	for(std::vector<VobLightInfo*>::iterator itv = lights.begin(); itv != lights.end();itv++)
	{
		zCVobLight* vob = (*itv)->Vob;

		// Reset state from CollectVisibleVobs
		(*itv)->VisibleInRenderPass = false;

		if(!vob->IsEnabled())
			continue;

		

		// Animate the light
		vob->DoAnimation();

		plcb.PL_Color = float4(vob->GetLightColor());
		plcb.PL_Range = vob->GetLightRange();
		plcb.Pl_PositionWorld = vob->GetPositionWorld();
		plcb.PL_Outdoor = (*itv)->IsIndoorVob ? 0.0f : 1.0f;

		float dist = D3DXVec3Length(&(*plcb.Pl_PositionWorld.toD3DXVECTOR3() - Engine::GAPI->GetCameraPosition()));

		// Gradually fade in the lights
		if( dist + plcb.PL_Range < Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius)
		{
			//float fadeStart = Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius - plcb.PL_Range;
			float fadeEnd = Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius;

			float fadeFactor = std::min(1.0f, std::max(0.0f, ((fadeEnd - (dist + plcb.PL_Range)) / plcb.PL_Range)));
			plcb.PL_Color.x *= fadeFactor;
			plcb.PL_Color.y *= fadeFactor;
			plcb.PL_Color.z *= fadeFactor;
			//plcb.PL_Color.w *= fadeFactor;
		}

		// Make the lights a little bit brighter
		plcb.PL_Color.x *= 1.5f;
		plcb.PL_Color.y *= 1.5f;
		plcb.PL_Color.z *= 1.5f;

		//if(plcb.PL_Range > 10000.0f)
		//	continue;

		//zTBBox3D bb = (*itv)->Vob->GetBBoxLocal();
		//GetLineRenderer()->AddAABBMinMax(bb.Min + (*itv)->Vob->GetPositionWorld() * 0.5f, bb.Max + (*itv)->Vob->GetPositionWorld() * 0.5f, *(D3DXVECTOR4 *)&float4((*itv)->Vob->GetLightColor()));
		//GetLineRenderer()->AddPointLocator(*plcb.Pl_PositionWorld.toD3DXVECTOR3(), 30.0f, *(D3DXVECTOR4 *)&float4((*itv)->Vob->GetLightColor()));

		// Need that in view space
		D3DXVec3TransformCoord(plcb.Pl_PositionView.toD3DXVECTOR3(), plcb.Pl_PositionWorld.toD3DXVECTOR3(), &view);
		D3DXVec3TransformCoord(plcb.PL_LightScreenPos.toD3DXVECTOR3(), plcb.Pl_PositionWorld.toD3DXVECTOR3(), &Engine::GAPI->GetProjectionMatrix());

		if(dist < plcb.PL_Range)
		{
			if(Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled)
			{
				Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled	= false;
				Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();
				Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
				UpdateRenderStates();
			}
		}else
		{
			if(!Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled)
			{
				Engine::GAPI->GetRendererState()->DepthState.DepthBufferEnabled	= true;
				Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
				Engine::GAPI->GetRendererState()->DepthState.SetDirty();
				Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
				UpdateRenderStates();
			}
		}

		plcb.PL_LightScreenPos.x = plcb.PL_LightScreenPos.x/2.0f +0.5f;
		plcb.PL_LightScreenPos.y = plcb.PL_LightScreenPos.y/-2.0f +0.5f;

		// Apply the constantbuffer to vs and PS
		ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&plcb);
		ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);
		ActivePS->GetConstantBuffer()[0]->BindToVertexShader(1); // Bind this instead of the usual per-instance buffer

		// Draw the mesh
		InverseUnitSphereMesh->DrawMesh();

		Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnLights++;
	}

	if(Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetBspTreeMode() == zBSP_MODE_INDOOR)
		return XR_SUCCESS; // No sunlight for indoor worlds

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	// Draw atmospheric scattering
	SetActivePixelShader("PS_DS_AtmosphericScattering");
	//SetActivePixelShader("PS_DS_SimpleSunlight");
	SetActiveVertexShader("VS_PFX");

	SetupVS_ExMeshDrawCall();

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	DS_ScreenQuadConstantBuffer scb;
	scb.SQ_InvProj = plcb.PL_InvProj;
	scb.SQ_InvView = plcb.PL_InvView;
	D3DXVec3TransformNormal(scb.SQ_LightDirectionVS.toD3DXVECTOR3(), sky->GetAtmosphereCB().AC_LightPos.toD3DXVECTOR3(), &view);

	float3 sunColor = Engine::GAPI->GetRendererState()->RendererSettings.SunLightColor;
	scb.SQ_LightColor = float4(sunColor.x, sunColor.y, sunColor.z,Engine::GAPI->GetRendererState()->RendererSettings.SunLightStrength);

	scb.SQ_ShadowView = cr.ViewReplacement;
	scb.SQ_ShadowProj = cr.ProjectionReplacement;
	scb.SQ_ShadowmapSize = (float)WorldShadowmap1->GetSizeX();

	//scb.SQ_ProjAB.x = Engine::GAPI->GetFarPlane() / (Engine::GAPI->GetFarPlane() - Engine::GAPI->GetNearPlane());
	//scb.SQ_ProjAB.y = (-Engine::GAPI->GetFarPlane() * Engine::GAPI->GetNearPlane()) / (Engine::GAPI->GetFarPlane() - Engine::GAPI->GetNearPlane());
	
	scb.SQ_ShadowStrength = Engine::GAPI->GetRendererState()->RendererSettings.ShadowStrength;
	scb.SQ_ShadowAOStrength = Engine::GAPI->GetRendererState()->RendererSettings.ShadowAOStrength;
	scb.SQ_WorldAOStrength = Engine::GAPI->GetRendererState()->RendererSettings.WorldAOStrength;

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&scb);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	PFXVS_ConstantBuffer vscb;
	vscb.PFXVS_InvProj = scb.SQ_InvProj;
	ActiveVS->GetConstantBuffer()[0]->UpdateBuffer(&vscb);
	ActiveVS->GetConstantBuffer()[0]->BindToVertexShader(0);

	WorldShadowmap1->BindToPixelShader(Context, 3);

	Context->PSSetSamplers(2,1, &ShadowmapSamplerState);

	PfxRenderer->DrawFullScreenQuad();



	// Reset state
	ID3D11ShaderResourceView* srv = NULL;
	Context->PSSetShaderResources(2, 1, &srv);
	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());


	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	UpdateRenderStates();
	return XR_SUCCESS;
}

/** Renders the shadowmaps for the sun */
void D3D11GraphicsEngine::RenderShadowmaps(const D3DXVECTOR3& cameraPosition)
{
	D3D11_VIEWPORT oldVP;
	UINT n = 1;
	Context->RSGetViewports(&n, &oldVP);

	// Apply new viewport
	D3D11_VIEWPORT vp;
	vp.TopLeftY = 0;
	vp.TopLeftY = 0;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = (float)WorldShadowmap1->GetSizeX();
	vp.Height = (float)WorldShadowmap1->GetSizeX();
	Context->RSSetViewports(1, &vp);

	// Set the rendering stage
	D3D11ENGINE_RENDER_STAGE oldStage = RenderingStage;
	SetRenderingStage(DES_SHADOWMAP);

	// Clear and Bind the shadowmap
	
	//Context->ClearRenderTargetView(GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetView(), (float *)&D3DXVECTOR4(1,0,0,0));

	ID3D11ShaderResourceView* srv = NULL;
	Context->PSSetShaderResources(3,1,&srv);
	Context->OMSetRenderTargets(0, NULL, WorldShadowmap1->GetDepthStencilView());
	//Context->OMSetRenderTargets(1, GBuffer1_Normals_SpecIntens_SpecPower->GetRenderTargetViewPtr(), WorldShadowmap1->GetDepthStencilView());

	//Engine::GAPI->SetFarPlane(20000.0f);

	// Dont render shadows from the sun when it isn't on the sky
	if (Engine::GAPI->GetSky()->GetAtmoshpereSettings().LightDirection.y > 0 && 
		Engine::GAPI->GetRendererState()->RendererSettings.DrawShadowGeometry &&
		Engine::GAPI->GetRendererState()->RendererSettings.EnableShadows)
	{
		Context->ClearDepthStencilView(WorldShadowmap1->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Draw the world mesh without textures
		DrawWorldAround(cameraPosition, 2, 10000.0f);
	}else
	{
		if(Engine::GAPI->GetSky()->GetAtmoshpereSettings().LightDirection.y <= 0)
		{
			Context->ClearDepthStencilView(WorldShadowmap1->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 0.0f, 0); // Always shadow in the night
		}else
		{
			Context->ClearDepthStencilView(WorldShadowmap1->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0); // Clear shadowmap when shadows not enabled
		}
	}

	// Restore state
	SetRenderingStage(oldStage);
	Context->RSSetViewports(1, &oldVP);

	Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);
}

/** Draws a fullscreenquad, copying the given texture to the viewport */
void D3D11GraphicsEngine::DrawQuad(INT2 position, INT2 size)
{
	ID3D11ShaderResourceView* srv;
	Context->PSGetShaderResources(0, 1, &srv);

	ID3D11RenderTargetView* rtv;
	Context->OMGetRenderTargets(1, &rtv, NULL);

	if(srv)
	{
		if(rtv)
		{
			PfxRenderer->CopyTextureToRTV(srv, rtv, size, false, position);
			rtv->Release();
		}
		srv->Release();
	}
}


/** Sets the current rendering stage */
void D3D11GraphicsEngine::SetRenderingStage(D3D11ENGINE_RENDER_STAGE stage)
{
	RenderingStage = stage;
}

/** Returns the current rendering stage */
D3D11ENGINE_RENDER_STAGE D3D11GraphicsEngine::GetRenderingStage()
{
	return RenderingStage;
}

/** Draws a single VOB */
void D3D11GraphicsEngine::DrawVobSingle(VobInfo* vob)
{
	//vob->UpdateVobConstantBuffer();
	//vob->VobConstantBuffer->BindToVertexShader(1);

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	SetActivePixelShader("PS_Preview_Textured");
	SetActiveVertexShader("VS_Ex");

	SetDefaultStates();
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();
	//SetupVS_ExPerInstanceConstantBuffer();
	//vob->VobConstantBuffer->BindToVertexShader(1);

	//D3DXMATRIX id;
	//D3DXMatrixIdentity(&id);
	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(vob->Vob->GetWorldMatrixPtr());
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

	InfiniteRangeConstantBuffer->BindToPixelShader(3);

	for(std::map<zCMaterial *, std::vector<MeshInfo*>>::iterator itm = vob->VisualInfo->Meshes.begin(); itm != vob->VisualInfo->Meshes.end();itm++)
	{
		for(unsigned int i=0;i<(*itm).second.size();i++)
		{
			// Cache texture
			if((*itm).first)
			{
				if((*itm).first->GetTexture())
				{
					if((*itm).first->GetTexture()->CacheIn(0.6f) == zRES_CACHED_IN)
					{
						(*itm).first->GetTexture()->Bind(0);

						MaterialInfo* info = Engine::GAPI->GetMaterialInfoFrom((*itm).first->GetTexture());
						if(!info->Constantbuffer)
						info->UpdateConstantbuffer();

						info->Constantbuffer->BindToPixelShader(2);
					}
					else
						continue;
				}
			}

			// Draw instances
			Engine::GraphicsEngine->DrawVertexBufferIndexed((*itm).second[i]->MeshVertexBuffer, (*itm).second[i]->MeshIndexBuffer, (*itm).second[i]->Indices.size());
		}
	}

	Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), NULL);
}

/** Message-Callback for the main window */
LRESULT D3D11GraphicsEngine::OnWindowMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	/*switch(msg)
	{
	case WM_KEYDOWN:
		switch(wParam)
		{
		
		}
		break;
	}*/

	if(UIView)UIView->OnWindowMessage(hWnd, msg, wParam, lParam);

	return 0;
}

/** Draws the ocean */
XRESULT D3D11GraphicsEngine::DrawOcean(GOcean* ocean)
{
	SetDefaultStates();

	// Then draw the ocean
	SetActivePixelShader("PS_Ocean");
	SetActiveVertexShader("VS_ExDisplace");

	// Set constant buffer
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GSky* sky = Engine::GAPI->GetSky();
	ActivePS->GetConstantBuffer()[1]->UpdateBuffer(&sky->GetAtmosphereCB());
	ActivePS->GetConstantBuffer()[1]->BindToPixelShader(1);

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = !Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise;
	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeWorld)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = true;
	}
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	// Init drawcalls
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();


	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	D3D11HDShader* hd = ShaderManager->GetHDShader("OceanTess");
	if(hd)hd->Apply();

	DefaultHullShaderConstantBuffer hscb;

	// convert to EdgesPerScreenHeight
	hscb.H_EdgesPerScreenHeight = GetResolution().y / Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
	hscb.H_Proj11 = Engine::GAPI->GetRendererState()->TransformState.TransformProj._22;
	hscb.H_GlobalTessFactor = Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
	hscb.H_ScreenResolution = float2(GetResolution().x, GetResolution().y);
	hscb.H_FarPlane = Engine::GAPI->GetFarPlane();
	hd->GetConstantBuffer()[0]->UpdateBuffer(&hscb);
	hd->GetConstantBuffer()[0]->BindToHullShader(1);


	

	ID3D11ShaderResourceView* tex_displacement;
	ID3D11ShaderResourceView* tex_gradient;
	ID3D11ShaderResourceView* tex_fresnel;
	ID3D11ShaderResourceView* cube_reflect = ReflectionCube;
	OceanSettingsConstantBuffer ocb;
	ocean->GetFFTResources(&tex_displacement, &tex_gradient, &tex_fresnel, &ocb);
	ocb.OS_SunColor = Engine::GAPI->GetSky()->GetSunColor();


	if(tex_gradient)
		Context->PSSetShaderResources(0,1, &tex_gradient);

	if(tex_displacement)
	{
		Context->DSSetShaderResources(0,1, &tex_displacement);
	}

	Context->PSSetShaderResources(1,1, &tex_fresnel);
	Context->PSSetShaderResources(3,1, &cube_reflect);

	// Scene information is still bound from rendering water surfaces

	Context->PSSetSamplers(1, 1, &ClampSamplerState);
	Context->PSSetSamplers(2, 1, &CubeSamplerState);

	// Update constantbuffer
	ActivePS->GetConstantBuffer()[2]->UpdateBuffer(&ocb);
	ActivePS->GetConstantBuffer()[2]->BindToPixelShader(4);

	//DistortionTexture->BindToPixelShader(0);

	RefractionInfoConstantBuffer ricb;
	ricb.RI_Projection = Engine::GAPI->GetProjectionMatrix();
	ricb.RI_ViewportSize = float2(Resolution.x, Resolution.y);
	ricb.RI_Time = Engine::GAPI->GetTimeSeconds();
	ricb.RI_CameraPosition = Engine::GAPI->GetCameraPosition();

	ActivePS->GetConstantBuffer()[4]->UpdateBuffer(&ricb);
	ActivePS->GetConstantBuffer()[4]->BindToPixelShader(2);

	// Bind distortion texture
	DistortionTexture->BindToPixelShader(4);

	// Bind copied backbuffer
	Context->PSSetShaderResources(5, 1, PfxRenderer->GetTempBuffer()->GetShaderResViewPtr());

	// Unbind depth buffer
	//Context->OMSetRenderTargets(1, HDRBackBuffer->GetRenderTargetViewPtr(), NULL);

	// Bind depth to the shader
	DepthStencilBufferCopy->BindToPixelShader(Context, 2);

	std::vector<D3DXVECTOR3> patches;
	ocean->GetPatchLocations(patches);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	D3DXMatrixTranspose(&view, &view);

	for(unsigned int i=0;i<patches.size();i++)
	{
		D3DXMATRIX scale, world;
		D3DXMatrixIdentity(&scale);
		D3DXMatrixScaling(&scale, (float)OCEAN_PATCH_SIZE, (float)OCEAN_PATCH_SIZE, (float)OCEAN_PATCH_SIZE);
		D3DXMatrixTranslation(&world, patches[i].x, patches[i].y, patches[i].z);
		world = scale * world;
		D3DXMatrixTranspose(&world, &world);
		ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&world);
		ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);

		
		D3DXMATRIX invWorldView;
		D3DXVECTOR3 localEye = D3DXVECTOR3(0,0,0);
		D3DXMatrixTranspose(&world, &world);

		D3DXMatrixInverse(&invWorldView, NULL, &(world * view));
		D3DXVec3TransformCoord(&localEye, &localEye, &invWorldView);

		OceanPerPatchConstantBuffer opp;
		opp.OPP_LocalEye = localEye;
		opp.OPP_PatchPosition = patches[i];
		ActivePS->GetConstantBuffer()[3]->UpdateBuffer(&opp);
		ActivePS->GetConstantBuffer()[3]->BindToPixelShader(3);

		

		ocean->GetPlaneMesh()->DrawMesh();
	}

	if(Engine::GAPI->GetRendererState()->RendererSettings.WireframeWorld)
	{
		Engine::GAPI->GetRendererState()->RasterizerState.Wireframe = false;
	}

	Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise = !Engine::GAPI->GetRendererState()->RasterizerState.FrontCounterClockwise;
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Context->PSSetSamplers(2, 1, &ShadowmapSamplerState);

	SetActivePixelShader("PS_World");
	SetActiveVertexShader("VS_Ex");

	Context->HSSetShader(NULL, NULL, 0);
	Context->DSSetShader(NULL, NULL, 0);
	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return XR_SUCCESS;
}

/** Constructs the makro list for shader compilation */
void D3D11GraphicsEngine::ConstructShaderMakroList(std::vector<D3D10_SHADER_MACRO>& list)
{
	GothicRendererSettings& s = Engine::GAPI->GetRendererState()->RendererSettings;
	D3D10_SHADER_MACRO m;

	m.Name = "SHD_ENABLE";
	m.Definition = s.EnableShadows ? "1" : "0";
	list.push_back(m);

	m.Name = "SHD_FILTER_16TAP_PCF";
	m.Definition = s.EnableSoftShadows ? "1" : "0";
	list.push_back(m);

	m.Name = NULL;
	m.Definition = NULL;
	list.push_back(m);
}

/** Handles an UI-Event */
void D3D11GraphicsEngine::OnUIEvent(EUIEvent uiEvent)
{
	if(uiEvent == UI_OpenSettings)
	{
		if(!UIView)
		{
			UIView = new D2DView;

			ID3D11Texture2D* tex;
			BackbufferRTV->GetResource((ID3D11Resource **)&tex);
			if(XR_SUCCESS != UIView->Init(Resolution, tex))
			{
				delete UIView;
				UIView = NULL;
			}

			if(tex)
				tex->Release();
		}
	
		// Show settings
		UIView->GetSettingsDialog()->SetHidden(!UIView->GetSettingsDialog()->IsHidden());

		// Free mouse
		Engine::GAPI->SetEnableGothicInput(UIView->GetSettingsDialog()->IsHidden());
	}
}

/** Returns the data of the backbuffer */
void D3D11GraphicsEngine::GetBackbufferData(byte** data, int& pixelsize)
{
	byte* d = new byte[GetBackbufferResolution().x * GetBackbufferResolution().y * 4];

	// Copy HDR scene to backbuffer
	SetDefaultStates();
	SetActivePixelShader("PS_PFX_GammaCorrectInv");
	ActivePS->Apply();

	GammaCorrectConstantBuffer gcb;
	gcb.G_Gamma = Engine::GAPI->GetGammaValue();
	gcb.G_Brightness = Engine::GAPI->GetBrightnessValue();

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&gcb);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), BackbufferRTV, INT2(0,0), true);
	
	HRESULT hr;
	ID3D11Resource *backbufferRes;
	BackbufferRTV->GetResource(&backbufferRes);

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.ArraySize = 1;
	texDesc.BindFlags = 0;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width = GetBackbufferResolution().x;  // must be same as backbuffer
	texDesc.Height = GetBackbufferResolution().y; // must be same as backbuffer
	texDesc.MipLevels = 1;
	texDesc.MiscFlags = 0;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_STAGING;

	ID3D11Texture2D *texture;
	LE(Device->CreateTexture2D(&texDesc, 0, &texture));
	Context->CopyResource(texture, backbufferRes);

	// Get data
	D3D11_MAPPED_SUBRESOURCE res;
	if(SUCCEEDED(Context->Map(texture, 0, D3D11_MAP_READ, 0, &res)))
	{
		memcpy(d, res.pData, GetBackbufferResolution().x * GetBackbufferResolution().y * 4);
		Context->Unmap(texture, 0);
	}

	pixelsize = 4;
	*data = d;

	texture->Release();
}

/** Binds the right shader for the given texture */
void D3D11GraphicsEngine::BindShaderForTexture(zCTexture* texture, bool forceAlphaTest, int zMatAlphaFunc)
{
	D3D11PShader* active = ActivePS;
	D3D11PShader* newShader = ActivePS;

	bool blendAdd = zMatAlphaFunc == zMAT_ALPHA_FUNC_ADD;
	bool blendBlend = zMatAlphaFunc == zMAT_ALPHA_FUNC_BLEND;

	if(blendAdd || blendBlend)
	{
		newShader = PS_Simple;
	}else if(texture->HasAlphaChannel() || forceAlphaTest)
	{
		if(texture->GetSurface()->GetNormalmap())
		{
			if(texture->GetSurface()->GetFxMap())
			{
				newShader = PS_DiffuseNormalmappedAlphatestFxMap;
			}else
			{
				newShader = PS_DiffuseNormalmappedAlphatest;
			}
		}else
		{
			newShader = PS_DiffuseAlphatest;
		}
	}else
	{
		if(texture->GetSurface()->GetNormalmap())
		{
			if(texture->GetSurface()->GetFxMap())
			{
				newShader = PS_DiffuseNormalmappedFxMap;
			}else
			{
				newShader = PS_DiffuseNormalmapped;
			}
		}else
		{
			newShader = PS_Diffuse;
		}
	}

	// Bind, if changed
	if(active != newShader)
	{
		ActivePS = newShader;
		ActivePS->Apply();
	}
}

/** Draws the given list of decals */
void D3D11GraphicsEngine::DrawDecalList(const std::vector<zCVob *>& decals, bool lighting)
{
	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = false;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	D3DXVECTOR3 camBack = -Engine::GAPI->GetCameraForward();

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);

	// Set up alpha
	if(!lighting)
	{
		SetActivePixelShader("PS_Simple");
		Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
	}else
	{
		SetActivePixelShader("PS_DiffuseAlphaTest");
	}
	
	SetActiveVertexShader("VS_Decal");
	
	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	int lastAlphaFunc = -1;
	for(unsigned int i=0;i<decals.size();i++)
	{
		zCDecal* d = (zCDecal *)decals[i]->GetVisual();

		if(lighting && !d->GetAlphaTestEnabled())
			continue; // Only allow no alpha or alpha test

		if(!lighting)
		{
			switch(d->GetDecalSettings()->DecalMaterial->GetAlphaFunc())
			{
			case zMAT_ALPHA_FUNC_BLEND:
				Engine::GAPI->GetRendererState()->BlendState.SetDefault();
				Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;	
				break;

			case zMAT_ALPHA_FUNC_ADD:
				Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
				break;

			default:
				continue; // FIXME: Draw Modulate!
			}

			if(lastAlphaFunc != d->GetDecalSettings()->DecalMaterial->GetAlphaFunc())
			{
				Engine::GAPI->GetRendererState()->BlendState.SetDirty();
				UpdateRenderStates();
				lastAlphaFunc = d->GetDecalSettings()->DecalMaterial->GetAlphaFunc();
			}
		}

		
		int alignment = decals[i]->GetAlignment();

		D3DXMATRIX world;
		/*if(alignment == zVISUAL_CAM_ALIGN_YAW)
		{
			D3DXVECTOR3 up = D3DXVECTOR3(0,1,0);
			D3DXVECTOR3 side; D3DXVec3Cross(&side, &up, &camBack);
			D3DXVECTOR3 forward; D3DXVec3Cross(&forward, &side, &up);
 
			D3DXVECTOR4 m[] = {D3DXVECTOR4(side, 0), D3DXVECTOR4(up,0), D3DXVECTOR4(forward,0), D3DXVECTOR4(decals[i]->GetPositionWorld(), 1)};
			world = *(D3DXMATRIX *)m;
		}else*/
		{
			//D3DXMatrixIdentity(&world);
			decals[i]->GetWorldMatrix(&world);
			//D3DXVECTOR3 wp = decals[i]->GetPositionWorld();
			//D3DXMatrixTranslation(&world, wp.x, wp.y, wp.z);
		}

		D3DXMATRIX offset; D3DXMatrixTranslation(&offset, d->GetDecalSettings()->DecalOffset.x,
			-d->GetDecalSettings()->DecalOffset.y, 0);

		D3DXMATRIX scale; D3DXMatrixScaling(&scale, d->GetDecalSettings()->DecalSize.x * 2, -d->GetDecalSettings()->DecalSize.y * 2, 1);
		D3DXMatrixTranspose(&scale, &scale);
		

		//D3DXMatrixTranspose(&world, &world);
		D3DXMATRIX mat = view * world;

		if(alignment == zVISUAL_CAM_ALIGN_FULL || alignment == zVISUAL_CAM_ALIGN_YAW) 
		{
			for(int x=0;x<3;x++)
			{
				for(int y=0;y<3;y++) 
				{
					if ( x==y )
						mat(x,y) = 1.0;
					else
						mat(x,y) = 0.0;
				}
			}
		}

		mat = mat * offset * scale;

		//D3DXMatrixTranspose(&mat, &mat);

		//DrawVertexBufferIndexed(QuadVertexBuffer, QuadIndexBuffer, 6);

		ParticleInstanceInfo ii;
		ii.worldview = mat;
		ii.scale = D3DXVECTOR2(50,50);
		ii.color = 0xFFFFFFFF;

		Engine::GAPI->SetWorldTransform(mat);
		SetupVS_ExPerInstanceConstantBuffer();
		
		if(d->GetDecalSettings()->DecalMaterial)
		{
			if(d->GetDecalSettings()->DecalMaterial->GetTexture())
			{
				if(d->GetDecalSettings()->DecalMaterial->GetTexture()->CacheIn(0.6f) != zRES_CACHED_IN)
				{
					continue; // Don't render not cached surfaces
				}

				d->GetDecalSettings()->DecalMaterial->BindTexture(0);
			}
		}

		DrawVertexBufferIndexed(QuadVertexBuffer, QuadIndexBuffer, 6);
		//instances.push_back(ii);
	}

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_BACK;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->DepthState.DepthWriteEnabled = true;
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;	
}


/** Draws quadmarks in a simple way */
void D3D11GraphicsEngine::DrawQuadMarks()
{
	D3DXVECTOR3 camPos = Engine::GAPI->GetCameraPosition();
	const stdext::hash_map<zCQuadMark*, QuadMarkInfo>& quadMarks = Engine::GAPI->GetQuadMarks();

	SetActiveVertexShader("VS_Ex");
	SetActivePixelShader("PS_World");

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	//Engine::GAPI->GetRendererState()->BlendState.SetModulateBlending();
	//Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	D3DXMATRIX view;
	Engine::GAPI->GetViewMatrix(&view);
	Engine::GAPI->SetViewTransform(view);

	SetupVS_ExMeshDrawCall();
	SetupVS_ExConstantBuffer();

	int alphaFunc = 0;
	for(stdext::hash_map<zCQuadMark*, QuadMarkInfo>::const_iterator it = quadMarks.begin(); it != quadMarks.end(); it++)
	{
		if(!(*it).first->GetConnectedVob())
			continue;

		if(D3DXVec3Length(&(camPos - (*it).second.Position)) > Engine::GAPI->GetRendererState()->RendererSettings.VisualFXDrawRadius)
			continue;

		zCMaterial* mat = (*it).first->GetMaterial();
		if(mat)
			mat->BindTexture(0);

		if(alphaFunc != mat->GetAlphaFunc())
		{
			// Change alpha-func		
			switch(mat->GetAlphaFunc())
			{
			case zMAT_ALPHA_FUNC_ADD:
				Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
				break;

			case zMAT_ALPHA_FUNC_BLEND:
				Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();
				break;

			case zMAT_ALPHA_FUNC_TEST:
				Engine::GAPI->GetRendererState()->BlendState.SetDefault();
				break;

			default:
				continue; // FIXME: Support modulate!
			}

			alphaFunc = mat->GetAlphaFunc();

			Engine::GAPI->GetRendererState()->BlendState.SetDirty();
			UpdateRenderStates();
		}

		Engine::GAPI->SetWorldTransform(*(*it).first->GetConnectedVob()->GetWorldMatrixPtr());
		SetupVS_ExPerInstanceConstantBuffer();

		Engine::GraphicsEngine->DrawVertexBuffer((*it).second.Mesh, (*it).second.NumVertices);
	}
}

/** Copies the depth stencil buffer to DepthStencilBufferCopy */
void D3D11GraphicsEngine::CopyDepthStencil()
{
	Context->CopyResource(DepthStencilBufferCopy->GetTexture(), DepthStencilBuffer->GetTexture());
}

/** Draws underwater effects */
void D3D11GraphicsEngine::DrawUnderwaterEffects()
{
	SetDefaultStates();

	RefractionInfoConstantBuffer ricb;
	ricb.RI_Projection = Engine::GAPI->GetProjectionMatrix();
	ricb.RI_ViewportSize = float2(Resolution.x, Resolution.y);
	ricb.RI_Time = Engine::GAPI->GetTimeSeconds();
	ricb.RI_CameraPosition = Engine::GAPI->GetCameraPosition();

	// Set up water final copy
	SetActivePixelShader("PS_PFX_UnderwaterFinal");
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&ricb);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(3);

	DistortionTexture->BindToPixelShader(2);
	DepthStencilBufferCopy->BindToPixelShader(Context, 3);

	PfxRenderer->BlurTexture(HDRBackBuffer, false, 1.0f, UNDERWATER_COLOR_MOD, "PS_PFX_UnderwaterFinal");
}

/** Creates the main UI-View */
void D3D11GraphicsEngine::CreateMainUIView()
{
	if(!UIView)
	{
		UIView = new D2DView;

		ID3D11Texture2D* tex;
		BackbufferRTV->GetResource((ID3D11Resource **)&tex);
		if(XR_SUCCESS != UIView->Init(Resolution, tex))
		{
			delete UIView;
			UIView = NULL;
		}

		if(tex)
			tex->Release();
	}
}

/** Sets up everything for a PNAEN-Mesh */
void D3D11GraphicsEngine::Setup_PNAEN(EPNAENRenderMode mode)
{
	D3D11HDShader* pnaen = ShaderManager->GetHDShader("PNAEN_Tesselation");

	if(mode == PNAEN_Instanced)
		SetActiveVertexShader("VS_PNAEN_Instanced");
	else if(mode == PNAEN_Default)
		SetActiveVertexShader("VS_PNAEN");
	else if(mode == PNAEN_Skeletal)
		SetActiveVertexShader("VS_PNAEN_Skeletal");

	ActiveVS->Apply();

	ActiveHDS = pnaen;
	pnaen->Apply();

	PNAENConstantBuffer cb;
	cb.f4Eye = Engine::GAPI->GetCameraPosition();
	cb.adaptive = INT4(0,0,0,0);
	cb.clipping = INT4(1,0,0,0);

	float f = Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor;
	cb.f4TessFactors = float4(f,f,Engine::GAPI->GetRendererState()->RendererSettings.TesselationRange, Engine::GAPI->GetRendererState()->RendererSettings.TesselationFactor);
	cb.f4ViewportScale.x = (float)GetResolution().x / 2;
	cb.f4ViewportScale.y = (float)GetResolution().y / 2;
	cb.f4x4Projection = Engine::GAPI->GetProjectionMatrix();

	pnaen->GetConstantBuffer()[0]->UpdateBuffer(&cb);

	pnaen->GetConstantBuffer()[0]->BindToDomainShader(0);
	pnaen->GetConstantBuffer()[0]->BindToHullShader(0);

	Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST);
}

/** Draws particle effects */
void D3D11GraphicsEngine::DrawFrameParticles(std::map<zCTexture*, std::vector<ParticleInstanceInfo>>& particles, std::map<zCTexture*, ParticleRenderInfo>& info)
{
	SetDefaultStates();

	// Copy scene behind the particle systems
	//PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), PfxRenderer->GetTempBuffer()->GetRenderTargetView());

	// Bind it to the second slot
	//PfxRenderer->GetTempBuffer()->BindToPixelShader(Context, 2);

	D3D11PShader* distPS = ShaderManager->GetPShader("PS_ParticleDistortion");

	RefractionInfoConstantBuffer ricb;
	ricb.RI_Projection = Engine::GAPI->GetProjectionMatrix();
	ricb.RI_ViewportSize = float2(Resolution.x, Resolution.y);
	ricb.RI_Time = Engine::GAPI->GetTimeSeconds();
	ricb.RI_CameraPosition = Engine::GAPI->GetCameraPosition();

	distPS->GetConstantBuffer()[0]->UpdateBuffer(&ricb);
	distPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GothicBlendStateInfo bsi = Engine::GAPI->GetRendererState()->BlendState;
	GothicRendererState& state = *Engine::GAPI->GetRendererState();

	state.BlendState.SetAdditiveBlending();
	state.BlendState.SetDirty();
	
	state.DepthState.DepthWriteEnabled = false;
	state.DepthState.SetDirty();
	state.DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_LESS_EQUAL;
	
	state.RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	state.RasterizerState.SetDirty();

	SetActivePixelShader("PS_Simple");
	PS_Simple->Apply();

	// Draw distortion for additive particles
	/*for(std::map<zCTexture*, std::vector<ParticleInstanceInfo>>::iterator it = particles.begin(); it != particles.end();it++)
	{
		ParticleRenderInfo& inf = info[(*it).first];
		GothicBlendStateInfo& blendState = inf.BlendState;

		if(inf.BlendMode == zRND_ALPHA_FUNC_ADD)
		{
			if((*it).first)
			{
				// Bind it
				if((*it).first->CacheIn(0.6f) == zRES_CACHED_IN)
					(*it).first->Bind(0);
				else
					continue;
			}

			state.BlendState.SetDefault();
			state.BlendState.SetDirty();

			distPS->Apply();

			DrawInstanced(vxb, ib, 6, &(*it).second[0], sizeof(ParticleInstanceInfo), (*it).second.size());

			PS_Simple->Apply();
		}
	}*/

	std::vector<std::tuple<zCTexture*, ParticleRenderInfo*, std::vector<ParticleInstanceInfo>*>> pvec;
	for(std::map<zCTexture*, std::vector<ParticleInstanceInfo>>::iterator it = particles.begin(); it != particles.end();it++)
	{
		if((*it).second.empty())
			continue;
		
		pvec.push_back(std::make_tuple((*it).first, &info[(*it).first], &(*it).second));
	}

	struct cmp
	{
		static bool cmppt(const std::tuple<zCTexture*, ParticleRenderInfo*, std::vector<ParticleInstanceInfo>*>& a,
						  const std::tuple<zCTexture*, ParticleRenderInfo*, std::vector<ParticleInstanceInfo>*>& b)
		{
			// Sort additive before blend
			return std::get<1>(a)->BlendMode > std::get<1>(b)->BlendMode;
		}
	};

	// Sort additive before blend
	std::sort(pvec.begin(), pvec.end(), cmp::cmppt);

	for(auto it = pvec.begin();it!=pvec.end();it++)
	{
		zCTexture* tx = std::get<0>((*it));
		ParticleRenderInfo& info = *std::get<1>((*it));
		std::vector<ParticleInstanceInfo>& instances = *std::get<2>((*it));

		if(instances.empty())
			continue;

		if(tx)
		{
			// Bind it
			if(tx->CacheIn(0.6f) == zRES_CACHED_IN)
				tx->Bind(0);
			else
				continue;
		}

		GothicBlendStateInfo& blendState = info.BlendState;

		// Setup blend state
		state.BlendState = blendState;
		state.BlendState.SetDirty();

		DrawInstanced(QuadVertexBuffer, QuadIndexBuffer, 6, &instances[0], sizeof(ParticleInstanceInfo), instances.size());
	}

	state.BlendState = bsi;
	state.BlendState.SetDirty();
	
	state.DepthState.DepthWriteEnabled = true;
	state.DepthState.SetDirty();
}

/** Called when a vob was removed from the world */
XRESULT D3D11GraphicsEngine::OnVobRemovedFromWorld(zCVob* vob)
{
	if(UIView)
		UIView->GetEditorPanel()->OnVobRemovedFromWorld(vob);

	return XR_SUCCESS;
}

/** Draws the particle-effects */
void D3D11GraphicsEngine::DrawParticleEffects()
{
	SetDefaultStates();

	// Copy scene behind the particle systems
	//PfxRenderer->CopyTextureToRTV(HDRBackBuffer->GetShaderResView(), PfxRenderer->GetTempBuffer()->GetRenderTargetView());

	// Bind it to the second slot
	//PfxRenderer->GetTempBuffer()->BindToPixelShader(Context, 2);

	D3D11PShader* distPS = ShaderManager->GetPShader("PS_ParticleDistortion");

	RefractionInfoConstantBuffer ricb;
	ricb.RI_Projection = Engine::GAPI->GetProjectionMatrix();
	ricb.RI_ViewportSize = float2(Resolution.x, Resolution.y);
	ricb.RI_Time = Engine::GAPI->GetTimeSeconds();
	ricb.RI_CameraPosition = Engine::GAPI->GetCameraPosition();

	distPS->GetConstantBuffer()[0]->UpdateBuffer(&ricb);
	distPS->GetConstantBuffer()[0]->BindToPixelShader(0);

	GothicBlendStateInfo bsi = Engine::GAPI->GetRendererState()->BlendState;
	GothicRendererState& state = *Engine::GAPI->GetRendererState();

	state.BlendState.SetAdditiveBlending();
	state.BlendState.SetDirty();
	
	state.DepthState.DepthWriteEnabled = false;
	state.DepthState.SetDirty();
	state.DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_LESS_EQUAL;
	
	state.RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	state.RasterizerState.SetDirty();

	SetActivePixelShader("PS_Simple");
	PS_Simple->Apply();

	D3D11VShader* vShader = ShaderManager->GetVShader("VS_ExInstanced");

	D3DXMATRIX& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	D3DXMATRIX& view = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	D3DXMATRIX& proj = Engine::GAPI->GetProjectionMatrix();

	
	VS_ExConstantBuffer_PerFrame cb;
	cb.View = view;
	cb.Projection = proj;

	VS_ExConstantBuffer_PerInstance cb2;
	cb2.World = world;

	vShader->GetConstantBuffer()[0]->UpdateBuffer(&cb);
	vShader->GetConstantBuffer()[0]->BindToVertexShader(0);

	//vShader->GetConstantBuffer()[1]->UpdateBuffer(&cb2);
	//vShader->GetConstantBuffer()[1]->BindToVertexShader(1);

	vShader->Apply();


	// Get visible particles
	std::vector<zCVob*> pfxList;
	std::map<zCTexture*, std::vector<zCVob*>> pfxByTexture;

	Engine::GAPI->GetVisibleParticleEffectsList(pfxList);

	// Sort them by texture (slow?)
	for(int i=0;i<pfxList.size();i++)
	{
		zCParticleFX* fx = (zCParticleFX*)pfxList[i]->GetVisual();

		pfxByTexture[fx->GetEmitter()->GetVisTexture()].push_back(pfxList[i]);
	}

	// Draw them
	for(auto it = pfxByTexture.begin(); it != pfxByTexture.end(); it++)
	{
		ParticleFrameData data;
		data.NeededSize = 0;
		data.BufferPosition = 0;

		DynamicInstancingBuffer->Map(BaseVertexBuffer::EMapFlags::M_WRITE_DISCARD, (void **)&data.Buffer, &data.BufferSize);
		
		// Fill the buffer with instances
		for(int i=0;i<(*it).second.size();i++)
		{
			zCParticleFX* fx = (zCParticleFX*)(*it).second[i]->GetVisual();
			
			Engine::GAPI->DrawParticleFX((*it).second[i], fx, data);
		}

		DynamicInstancingBuffer->Unmap();

		// Draw particles

		zCTexture* tx = (*it).first;
		ParticleRenderInfo& inf = Engine::GAPI->GetFrameParticleInfo()[tx]; // FIXME: This only contains one element!

		if(tx)
		{
			// Bind it
			if(tx->CacheIn(0.6f) == zRES_CACHED_IN)
				tx->Bind(0);
			else
				continue;
		}

		GothicBlendStateInfo& blendState = inf.BlendState;

		// Setup blend state
		state.BlendState = blendState;
		state.BlendState.SetDirty();
		UpdateRenderStates();

		// Draw particles
		DrawInstanced(QuadVertexBuffer, QuadIndexBuffer, 6, DynamicInstancingBuffer, sizeof(ParticleInstanceInfo), std::min(data.NeededSize, data.BufferSize) / sizeof(ParticleInstanceInfo));

		Engine::GAPI->GetFrameParticleInfo().clear(); // FIXME: This only contains one element!


		// Recreate for next frame if needed
		if(data.NeededSize > data.BufferSize)
		{
			LogInfo() << "Particle Instancing-Buffer too small (" << data.BufferSize << "), need " << data.NeededSize << " bytes. Recreating buffer.";

			// Buffer too small, recreate it
			delete DynamicInstancingBuffer;
			DynamicInstancingBuffer = new D3D11VertexBuffer();

			DynamicInstancingBuffer->Init(NULL, data.NeededSize, BaseVertexBuffer::B_VERTEXBUFFER, BaseVertexBuffer::U_DYNAMIC, BaseVertexBuffer::CA_WRITE);
		}
	}
}

